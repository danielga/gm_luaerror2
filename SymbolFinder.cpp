#include "SymbolFinder.hpp"
#include <stdint.h>
#include <map>
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif __linux
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <interface.h>
#elif __APPLE__
#include <mach/task.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <interface.h>
#endif

struct DynLibInfo
{
	void *baseAddress;
	size_t memorySize;
};

#if __linux || __APPLE__
struct LibSymbolTable
{
	std::map<const char *, void *> table;
	uintptr_t lib_base;
	uint32_t last_pos;
};
#endif

SymbolFinder::SymbolFinder( )
{
#if __APPLE__
	Gestalt( gestaltSystemVersionMajor, &m_OSXMajor );
	Gestalt( gestaltSystemVersionMinor, &m_OSXMinor );

	if( ( m_OSXMajor == 10 && m_OSXMinor >= 6 ) || m_OSXMajor > 10 )
	{
		task_dyld_info_data_t dyld_info;
		mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
		task_info( mach_task_self( ), TASK_DYLD_INFO, (task_info_t)&dyld_info, &count );
		m_ImageList = (struct dyld_all_image_infos *)dyld_info.all_image_info_addr;
	}
	else
	{
		struct nlist list[2];
		memset( list, 0, sizeof( list ) );
		list[0].n_un.n_name = (char *)"_dyld_all_image_infos";
		nlist( "/usr/lib/dyld", list );
		m_ImageList = (struct dyld_all_image_infos *)list[0].n_value;
	}
#endif
}

SymbolFinder::~SymbolFinder( )
{
#if __linux || __APPLE__
	for( size_t i = 0; i < symbolTables.size( ); ++i )
	{
		delete symbolTables[i];
	}

	symbolTables.clear( );
#endif
}

void *SymbolFinder::FindPattern( const void *handle, const char *pattern, const size_t &len )
{
	DynLibInfo lib;
	memset( &lib, 0, sizeof( DynLibInfo ) );
	if( !GetLibraryInfo( handle, lib ) )
	{
		return 0;
	}

	char *ptr = reinterpret_cast<char *>( lib.baseAddress );
	char *end = ptr + lib.memorySize - len;
	bool found = false;
	while( ptr < end )
	{
		found = true;
		for( size_t i = 0; i < len; ++i )
		{
			if( pattern[i] != '\x2A' && pattern[i] != ptr[i] )
			{
				found = false;
				break;
			}
		}

		if( found )
		{
			return ptr;
		}

		ptr++;
	}

	return 0;
}

void *SymbolFinder::FindSymbol( const void *handle, const char *symbol )
{
#if _WIN32
	return (void *)GetProcAddress( (HMODULE)handle, symbol );
#elif __linux
	struct link_map *dlmap = (struct link_map *)handle;
	LibSymbolTable *libtable = 0;
	std::map<const char *, void *> *table = 0;
	for( size_t i = 0; i < symbolTables.size( ); ++i )
	{
		libtable = symbolTables[i];
		if( libtable->lib_base == dlmap->l_addr )
		{
			table = &libtable->table;
			break;
		}
	}

	if( table == 0 )
	{
		libtable = new LibSymbolTable( );
		libtable->lib_base = dlmap->l_addr;
		libtable->last_pos = 0;
		table = &libtable->table;
		symbolTables.push_back( libtable );
	}

	void *symbol_entry = table[symbol];
	if( symbol_entry != 0 )
	{
		return symbol_entry;
	}

	struct stat dlstat;
	int dlfile = open( dlmap->l_name, O_RDONLY );
	if( dlfile == -1 || fstat( dlfile, &dlstat ) == -1 )
	{
		close( dlfile );
		return 0;
	}

	Elf32_Ehdr *file_hdr = (Elf32_Ehdr *)mmap( 0, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0 );
	uintptr_t map_base = (uintptr_t)file_hdr;
	close( dlfile );
	if( file_hdr == MAP_FAILED )
	{
		return 0;
	}

	if( file_hdr->e_shoff == 0 || file_hdr->e_shstrndx == SHN_UNDEF )
	{
		munmap( file_hdr, dlstat.st_size );
		return 0;
	}

	Elf32_Shdr *symtab_hdr = 0, *strtab_hdr = 0;
	Elf32_Shdr *sections = (Elf32_Shdr *)( map_base + file_hdr->e_shoff );
	uint16_t section_count = file_hdr->e_shnum;
	Elf32_Shdr *shstrtab_hdr = &sections[file_hdr->e_shstrndx];
	const char *shstrtab = (const char *)( map_base + shstrtab_hdr->sh_offset );
	for( uint16_t i = 0; i < section_count; i++ )
	{
		Elf32_Shdr &hdr = sections[i];
		const char *section_name = shstrtab + hdr.sh_name;
		if( strcmp(section_name, ".symtab") == 0 )
		{
			symtab_hdr = &hdr;
		}
		else if( strcmp( section_name, ".strtab" ) == 0 )
		{
			strtab_hdr = &hdr;
		}
	}

	if( symtab_hdr == 0 || strtab_hdr == 0 )
	{
		munmap( file_hdr, dlstat.st_size );
		return 0;
	}

	Elf32_Sym *symtab = (Elf32_Sym *)( map_base + symtab_hdr->sh_offset );
	const char *strtab = (const char *)( map_base + strtab_hdr->sh_offset );
	uint32_t symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
	void *symbol_pointer = 0;
	for( uint32_t i = libtable->last_pos; i < symbol_count; i++ )
	{
		Elf32_Sym &sym = symtab[i];
		unsigned char sym_type = ELF32_ST_TYPE( sym.st_info );
		const char *sym_name = strtab + sym.st_name;

		if( sym.st_shndx == SHN_UNDEF || ( sym_type != STT_FUNC && sym_type != STT_OBJECT ) )
		{
			continue;
		}

		table[sym_name] = (void *)( dlmap->l_addr + sym.st_value );
		if( strcmp( symbol, sym_name ) == 0 )
		{
			libtable->last_pos = ++i;
			symbol_pointer = (void *)( dlmap->l_addr + sym.st_value );
			break;
		}
	}

	munmap( file_hdr, dlstat.st_size );
	return symbol_pointer;
#elif __APPLE__
	uintptr_t dlbase = 0;
	uint32_t image_count = m_ImageList->infoArrayCount;
	struct segment_command *linkedit_hdr = 0;
	struct symtab_command *symtab_hdr = 0;
	for( uint32_t i = 1; i < image_count; i++ )
	{
		const struct dyld_image_info &info = m_ImageList->infoArray[i];
		void *h = dlopen( info.imageFilePath, RTLD_NOLOAD );
		if( h == handle )
		{
			dlbase = (uintptr_t)info.imageLoadAddress;
			dlclose( h );
			break;
		}

		dlclose( h );
	}

	if( !dlbase )
	{
		return 0;
	}

	LibSymbolTable *libtable = 0;
	std::map<const char *, void *> *table = 0;
	for( size_t i = 0; i < symbolTables.size( ); ++i )
	{
		libtable = symbolTables[i];
		if( libtable->lib_base == dlbase )
		{
			table = &libtable->table;
			break;
		}
	}

	if( table == 0 )
	{
		libtable = new LibSymbolTable( );
		libtable->lib_base = dlbase;
		libtable->last_pos = 0;
		table = &libtable->table;
		symbolTables.push_back( libtable );
	}

	void *symbol_entry = table[symbol];
	if( symbol_entry != 0 )
	{
		return symbol_entry;
	}

	struct mach_header *file_hdr = (struct mach_header *)dlbase;
	struct load_command *loadcmds = (struct load_command *)( dlbase + sizeof( struct mach_header ) );
	uint32_t loadcmd_count = file_hdr->ncmds;
	for( uint32_t i = 0; i < loadcmd_count; i++ )
	{
		if( loadcmds->cmd == LC_SEGMENT && !linkedit_hdr )
		{
			struct segment_command *seg = (struct segment_command *)loadcmds;
			if( strcmp( seg->segname, "__LINKEDIT" ) == 0 )
			{
				linkedit_hdr = seg;
				if( symtab_hdr )
				{
					break;
				}
			}
		}
		else if( loadcmds->cmd == LC_SYMTAB )
		{
			symtab_hdr = (struct symtab_command *)loadcmds;
			if( linkedit_hdr )
			{
				break;
			}
		}

		loadcmds = (struct load_command *)( (uintptr_t)loadcmds + loadcmds->cmdsize );
	}

	if( !linkedit_hdr || !symtab_hdr || !symtab_hdr->symoff || !symtab_hdr->stroff )
	{
		return 0;
	}

	uintptr_t linkedit_addr = dlbase + linkedit_hdr->vmaddr;
	struct nlist *symtab = (struct nlist *)( linkedit_addr + symtab_hdr->symoff - linkedit_hdr->fileoff );
	const char *strtab = (const char *)( linkedit_addr + symtab_hdr->stroff - linkedit_hdr->fileoff );
	uint32_t symbol_count = symtab_hdr->nsyms;
	void *symbol_pointer = 0;
	for( uint32_t i = libtable->last_pos; i < symbol_count; i++ )
	{
		struct nlist &sym = symtab[i];
		const char *sym_name = strtab + sym.n_un.n_strx + 1;
		if( sym.n_sect == NO_SECT )
		{
			continue;
		}

		table[sym_name] = (void *)( dlbase + sym.n_value );
		if( strcmp( symbol, sym_name ) == 0 )
		{
			libtable->last_pos = ++i;
			symbol_pointer = (void *)( dlbase + sym.n_value );
			break;
		}
	}

	return symbol_pointer;
#endif
}

void *SymbolFinder::FindSymbolFromBinary( const char *name, const char *symbol )
{
#if _WIN32
	HMODULE binary = 0;
	if( GetModuleHandleEx( 0, name, &binary ) == TRUE && binary != 0 )
	{
		void *symbol_pointer = FindSymbol( binary, symbol );
		FreeModule( binary );
		return symbol_pointer;
	}
#elif __linux || __APPLE__
	void *binary = dlopen( name, RTLD_NOW | RTLD_LOCAL );
	if( binary != 0 )
	{
		void *symbol_pointer = ResolveSymbol( binary, symbol );
		dlclose( binary );
		return symbol_pointer;
	}
#endif
	return 0;
}

void *SymbolFinder::Resolve( const void *handle, const char *data, const size_t &len )
{
	if( data[0] == '@' )
	{
		return FindSymbol( handle, data );
	}

	return FindPattern( handle, data, len );
}

void *SymbolFinder::ResolveOnBinary( const char *name, const char *data, const size_t &len )
{
#if _WIN32
	HMODULE binary = 0;
	if( GetModuleHandleEx( 0, name, &binary ) == TRUE && binary != 0 )
	{
		void *symbol_pointer = Resolve( binary, data, len );
		FreeModule( binary );
		return symbol_pointer;
	}
#elif __linux || __APPLE__
	void *binary = dlopen( name, RTLD_NOW | RTLD_LOCAL );
	if( binary != 0 )
	{
		void *symbol_pointer = Resolve( binary, data, len );
		dlclose( binary );
		return symbol_pointer;
	}
#endif
	return 0;
}

bool SymbolFinder::GetLibraryInfo( const void *handle, DynLibInfo &lib )
{
	if( handle == 0 )
	{
		return false;
	}

#if _WIN32
	MEMORY_BASIC_INFORMATION info;
	if( !VirtualQuery( handle, &info, sizeof( MEMORY_BASIC_INFORMATION ) ) )
	{
		return false;
	}

	uintptr_t baseAddr = reinterpret_cast<uintptr_t>( info.AllocationBase );

	IMAGE_DOS_HEADER *dos = reinterpret_cast<IMAGE_DOS_HEADER *>( baseAddr );
	IMAGE_NT_HEADERS *pe = reinterpret_cast<IMAGE_NT_HEADERS *>( baseAddr + dos->e_lfanew );
	IMAGE_FILE_HEADER *file = &pe->FileHeader;
	IMAGE_OPTIONAL_HEADER *opt = &pe->OptionalHeader;

	if( dos->e_magic != IMAGE_DOS_SIGNATURE || pe->Signature != IMAGE_NT_SIGNATURE || opt->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC )
	{
		return false;
	}

	if( file->Machine != IMAGE_FILE_MACHINE_I386 )
	{
		return false;
	}

	if( ( file->Characteristics & IMAGE_FILE_DLL ) == 0 )
	{
		return false;
	}

	lib.memorySize = opt->SizeOfImage;
#elif __linux
	Dl_info info;
	if( !dladdr( handle, &info ) )
	{
		return false;
	}

	if( !info.dli_fbase || !info.dli_fname )
	{
		return false;
	}

	uintptr_t baseAddr = reinterpret_cast<uintptr_t>(info.dli_fbase);
	Elf32_Ehdr *file = reinterpret_cast<Elf32_Ehdr *>(baseAddr);

	if( memcmp( ELFMAG, file->e_ident, SELFMAG ) != 0 )
	{
		return false;
	}

	if( file->e_ident[EI_VERSION] != EV_CURRENT )
	{
		return false;
	}

	if( file->e_ident[EI_CLASS] != ELFCLASS32 || file->e_machine != EM_386 || file->e_ident[EI_DATA] != ELFDATA2LSB )
	{
		return false;
	}

	if( file->e_type != ET_DYN )
	{
		return false;
	}

	uint16_t phdrCount = file->e_phnum;
	Elf32_Phdr *phdr = reinterpret_cast<Elf32_Phdr *>( baseAddr + file->e_phoff );

	for( uint16_t i = 0; i < phdrCount; ++i )
	{
		Elf32_Phdr &hdr = phdr[i];

		if( hdr.p_type == PT_LOAD && hdr.p_flags == ( PF_X | PF_R ) )
		{
			lib.memorySize = PAGE_ALIGN_UP( hdr.p_filesz );
			break;
		}
	}
#elif __APPLE__
	Dl_info info;
	if( !dladdr( handle, &info ) )
	{
		return false;
	}

	if( !info.dli_fbase || !info.dli_fname )
	{
		return false;
	}

	uintptr_t baseAddr = (uintptr_t)info.dli_fbase;
	struct mach_header *file = (struct mach_header *)baseAddr;

	if( file->magic != MH_MAGIC )
	{
		return false;
	}

	if( file->cputype != CPU_TYPE_I386 || file->cpusubtype != CPU_SUBTYPE_I386_ALL )
	{
		return false;
	}

	if( file->filetype != MH_DYLIB )
	{
		return false;
	}

	uint32_t cmd_count = file->ncmds;
	struct segment_command *seg = (struct segment_command *)( baseAddr + sizeof( struct mach_header ) );

	for( uint32_t i = 0; i < cmd_count; ++i )
	{
		if( seg->cmd == LC_SEGMENT )
		{
			lib.memorySize += seg->vmsize;
		}

		seg = (struct segment_command *)( (uintptr_t)seg + seg->cmdsize );
	}
#endif

	lib.baseAddress = reinterpret_cast<void *>( baseAddr );
	return true;
}
