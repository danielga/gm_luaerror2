#include <GarrysMod/Lua/Interface.h>
#include <string>

#include <utlvector.h>
#include <Color.h>

#include "MologieDetours/detours.h"
#if _WIN32
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include "csimplescan.h"
#elif __linux
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#elif __APPLE__
#include <mach/task.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

static CUtlVector<GarrysMod::Lua::ILuaBase *> luaList;

typedef struct lua_Debug
{
	int event;
	const char *name;			/* (n) */
	const char *namewhat;		/* (n) */
	const char *what;			/* (S) */
	const char *source;			/* (S) */
	int currentline;			/* (l) */
	int nups;					/* (u) number of upvalues */
	int linedefined;			/* (S) */
	int lastlinedefined;		/* (S) */
	char short_src[512];		/* (S) */
	/* private part */
	int i_ci;
} lua_Debug;

bool IsServer( GarrysMod::Lua::ILuaBase *lua )
{
	lua->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
	{
		lua->GetField( -1, "SERVER" );
		lua->Remove( -2 ); // Remove global table from stack
		if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( -1 ) )
		{
			lua->Pop( 1 );
			return true;
		}

		lua->Pop( 1 );
	}

	return false;
}

/*
#if _WIN32
#define lua_getstack_signature "\x51\x56\x8b\x74\x24\x10\x57\x8b\x7c\x24\x10\x8d\x44\x24\x08\x50"
#define lua_getstack_mask "xxxxxxxxxxxxxxxx"
#endif
*/
typedef int ( *lua_getstack_t ) ( lua_State *state, int level, lua_Debug *ar );
lua_getstack_t lua_getstack = 0;

/*
#if _WIN32
#define lua_getinfo_signature "\x8b\x44\x24\x0c\x8b\x4c\x24\x08\x8b\x54\x24\x04\x6a\x00\x50\x51"
#define lua_getinfo_mask "xxxxxxxxxxxxxxxx"
#endif
*/
typedef int ( *lua_getinfo_t ) ( lua_State *state, const char *what, lua_Debug *ar );
lua_getinfo_t lua_getinfo = 0;

bool CallErrorHook( GarrysMod::Lua::ILuaBase *lua, bool serverside, bool runtime, const char *error, CUtlVector<lua_Debug> &stack )
{
	lua->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
	{
		lua->GetField( -1, "hook" );
		lua->Remove( -2 ); // Remove global table from stack
		if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
		{
			lua->GetField( -1, "Run" );
			lua->Remove( -2 ); // Remove hook library table from stack
			if( lua->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
			{
				lua->PushString( "LuaError" );
				lua->PushBool( serverside );
				lua->PushBool( runtime );
				lua->PushString( error );

				if( runtime )
				{
					lua->CreateTable( );
					if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
					{
						for( int i = 0; i < stack.Count( ); i++ )
						{
							lua->PushNumber( (double)( i + 1 ) );
							lua->CreateTable( );
							if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
							{
								lua->PushNumber( (double)stack[i].event );
								lua->SetField( -2, "event" );

								lua->PushString( stack[i].name );
								lua->SetField( -2, "name" );

								lua->PushString( stack[i].namewhat );
								lua->SetField( -2, "namewhat" );

								lua->PushString( stack[i].what );
								lua->SetField( -2, "what" );

								lua->PushString( stack[i].source );
								lua->SetField( -2, "source" );

								lua->PushNumber( (double)stack[i].currentline );
								lua->SetField( -2, "currentline" );

								lua->PushNumber( (double)stack[i].nups );
								lua->SetField( -2, "nups" );

								lua->PushNumber( (double)stack[i].linedefined );
								lua->SetField( -2, "linedefined" );

								lua->PushNumber( (double)stack[i].lastlinedefined );
								lua->SetField( -2, "lastlinedefined" );

								lua->PushString( stack[i].short_src );
								lua->SetField( -2, "short_src" );

								lua->PushNumber( (double)stack[i].i_ci );
								lua->SetField( -2, "i_ci" );
							}

							lua->SetTable( -3 );
						}
					}

					if( lua->PCall( 5, 1, 0 ) != 0 )
					{
						ConColorMsg( Color( 255, 0, 0, 255 ), "[LuaError hook error] %s\n", lua->GetString( ) );
						return true;
					}
				}
				else
				{
					if( lua->PCall( 4, 1, 0 ) != 0 )
					{
						ConColorMsg( Color( 255, 0, 0, 255 ), "[LuaError hook error] %s\n", lua->GetString( ) );
						return true;
					}
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 1 );
					return false;
				}

				lua->Pop( 1 );
			}
		}
	}

	return true;
}

/*
#if _WIN32
#define luaL_loadbufferx_signature "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x78\x8B\x45\x0C\x8B\x4D\x10\x89"
#define luaL_loadbufferx_mask "xxxxxxxxxxxxxxxx"
#endif
*/
typedef int ( *luaL_loadbufferx_t ) ( lua_State *state, const char *buff, size_t size, const char *name, const char *mode );
MologieDetours::Detour<luaL_loadbufferx_t> *luaL_loadbufferx_detour = 0;
luaL_loadbufferx_t luaL_loadbufferx = 0;
int luaL_loadbufferx_d( lua_State *state, const char *buff, size_t size, const char *name, const char *mode )
{
	int error = luaL_loadbufferx_detour->GetOriginalFunction( )( state, buff, size, name, mode );
	if( error != 0 && LUA->IsType( -1, GarrysMod::Lua::Type::STRING ) )
	{
		const char *strerr = LUA->GetString( );

		CUtlVector<lua_Debug> stack;
		lua_Debug dbg = { };
		int level = 1;
		while( lua_getstack( state, level, &dbg ) == 1 )
		{
			if( lua_getinfo( state, "Slnu", &dbg ) == 0 )
			{
				break;
			}

			level++;
			stack.AddToTail( dbg );
			memset( &dbg, 0, sizeof( dbg ) );
		}

		for( int k = 0; k < luaList.Count( ); k++ )
		{
			GarrysMod::Lua::ILuaBase *lua = luaList.Element( k );
			if( lua != 0 && !CallErrorHook( lua, IsServer( LUA ), false, strerr, stack ) )
			{
				break;
			}
		}
	}

	return error;
}

#if _WIN32
#define AdvancedLuaErrorReporter_signature "\x55\x8b\xec\x56\x8b\x75\x08\x6a\x01\x56\xe8\x00\x00\x00\x00\x83\xc4\x08"
#define AdvancedLuaErrorReporter_mask "xxxxxxxxxxx????xxx"
#endif
typedef int ( *AdvancedLuaErrorReporter_t ) ( lua_State *state );
MologieDetours::Detour<AdvancedLuaErrorReporter_t> *AdvancedLuaErrorReporter_detour = 0;
AdvancedLuaErrorReporter_t AdvancedLuaErrorReporter = 0;
int AdvancedLuaErrorReporter_d( lua_State *state )
{
	int rets = AdvancedLuaErrorReporter_detour->GetOriginalFunction( )( state );
	if( rets > 0 && LUA->IsType( -1, GarrysMod::Lua::Type::STRING ) )
	{
		const char *strerr = LUA->GetString( -2 );

		CUtlVector<lua_Debug> stack;
		lua_Debug dbg = { };
		int level = 1;
		while( lua_getstack( state, level, &dbg ) == 1 )
		{
			if( lua_getinfo( state, "Slnu", &dbg ) == 0 )
			{
				break;
			}

			level++;
			stack.AddToTail( dbg );
			memset( &dbg, 0, sizeof( dbg ) );
		}

		for( int k = 0; k < luaList.Count( ); k++ )
		{
			GarrysMod::Lua::ILuaBase *lua = luaList.Element( k );
			if( lua != 0 && !CallErrorHook( lua, IsServer( LUA ), true, strerr, stack ) )
			{
				break;
			}
		}
	}

	return rets;
}

#if _WIN32
#define Push_Entity_signature "\x55\x8b\xec\x83\xec\x14\x83\x3d\x00\x00\x00\x00\x00\x74\x00\x8b\x4d\x08"
#define Push_Entity_mask "xxxxxxxx?????x?xxx"
#endif
class CBaseEntity;
class CBasePlayer;
typedef void ( *Push_Entity_t ) ( CBaseEntity *entity );
Push_Entity_t Push_Entity;

#if _WIN32
#define HandleClientLuaError_signature "\x55\x8b\xec\x83\xec\x08\x8b\x0d\x00\x00\x00\x00\x8b\x11\x53\x56"
#define HandleClientLuaError_mask "xxxxxxxx????xxxx"
#endif
typedef int ( *HandleClientLuaError_t ) ( CBasePlayer *player, const char *error );
MologieDetours::Detour<HandleClientLuaError_t> *HandleClientLuaError_detour = 0;
HandleClientLuaError_t HandleClientLuaError = 0;
int HandleClientLuaError_d( CBasePlayer *player, const char *error )
{
	for( int k = 0; k < luaList.Count( ); k++ )
	{
		GarrysMod::Lua::ILuaBase *lua = luaList.Element( k );
		if( lua != 0 && IsServer( lua ) )
		{
			lua->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
			if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
			{
				lua->GetField( -1, "hook" );
				lua->Remove( -2 ); // Remove global table from stack
				if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
				{
					lua->GetField( -1, "Run" );
					lua->Remove( -2 ); // Remove hook library table from stack
					if( lua->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
					{
						lua->PushString( "ClientLuaError" );
						Push_Entity( (CBaseEntity *)player );
						lua->PushString( error );

						if( lua->PCall( 3, 1, 0 ) != 0 )
						{
							ConColorMsg( Color( 255, 0, 0, 255 ), "[ClientLuaError hook error] %s\n", lua->GetString( ) );
							return 0;
						}

						if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
						{
							lua->Pop( 1 );
							return 0;
						}

						lua->Pop( 1 );
					}
				}
			}
		}
	}

	return HandleClientLuaError_detour->GetOriginalFunction( )( player, error );
}

#ifndef _WIN32
#if __APPLE__
struct dyld_all_image_infos *m_ImageList;
SInt32 m_OSXMajor;
SInt32 m_OSXMinor;
#endif
void *FindFunctionsTheHardWay( void *handle, const char *symbol )   // Totally not stolen from SourceMods
																	// Allows finding functions not exported
{
#if __linux
	struct link_map *dlmap = (struct link_map *)handle;
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
	for( uint32_t i = 0; i < symbol_count; i++ )
	{
		Elf32_Sym &sym = symtab[i];
		unsigned char sym_type = ELF32_ST_TYPE( sym.st_info );
		const char *sym_name = strtab + sym.st_name;

		if( sym.st_shndx == SHN_UNDEF || ( sym_type != STT_FUNC && sym_type != STT_OBJECT ) )
		{
			continue;
		}

		if( strcmp( symbol, sym_name ) == 0 )
		{
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
	for( uint32_t i = 0; i < symbol_count; i++ )
	{
		struct nlist &sym = symtab[i];
		const char *sym_name = strtab + sym.n_un.n_strx + 1;
		if( sym.n_sect == NO_SECT )
		{
			continue;
		}

		if( strcmp( symbol, sym_name ) == 0 )
		{
			symbol_pointer = (void *)( dlbase + sym.n_value );
			break;
		}
	}

	return symbol_pointer;
#endif
}
#endif

GMOD_MODULE_OPEN( )
{
	if( luaList.Count( ) == 0 )
	{
#if _WIN32
		HMODULE lua_shared = 0;
		BOOL success = GetModuleHandleEx( 0, "lua_shared.dll", &lua_shared );
		CSimpleScan lua_shared_scan;
		if( success == TRUE && lua_shared_scan.SetDLL( "lua_shared.dll" ) )
		{
			/*
			if( !lua_shared_scan.FindFunction( luaL_loadbufferx_signature, luaL_loadbufferx_mask, &(void *&)luaL_loadbufferx ) )
			{
				Msg( "[LuaError] Unable to detour function luaL_loadbufferx.\n" );
			}

			if( !lua_shared_scan.FindFunction( lua_getstack_signature, lua_getstack_mask, &(void *&)lua_getstack ) )
			{
				Msg( "[LuaError] Unable to scan function lua_getstack.\n" );
			}

			if( !lua_shared_scan.FindFunction( lua_getinfo_signature, lua_getinfo_mask, &(void *&)lua_getinfo ) )
			{
				Msg( "[LuaError] Unable to scan function lua_getinfo.\n" );
			}
			*/

			if( ( lua_getstack = (lua_getstack_t)GetProcAddress( lua_shared, "lua_getstack" ) ) == 0 )
			{
				Msg( "[LuaError] Unable to scan function lua_getstack.\n" );
			}

			if( ( lua_getinfo = (lua_getinfo_t)GetProcAddress( lua_shared, "lua_getinfo" ) ) == 0 )
			{
				Msg( "[LuaError] Unable to scan function lua_getinfo.\n" );
			}

			if( ( luaL_loadbufferx = (luaL_loadbufferx_t)GetProcAddress( lua_shared, "luaL_loadbufferx" ) ) == 0 )
			{
				Msg( "[LuaError] Unable to detour function luaL_loadbufferx.\n" );
			}

			if( !lua_shared_scan.FindFunction( AdvancedLuaErrorReporter_signature, AdvancedLuaErrorReporter_mask, &(void *&)AdvancedLuaErrorReporter ) )
			{
				Msg( "[LuaError] Unable to detour function AdvancedLuaErrorReporter.\n" );
			}
		}
		else
		{
			Msg( "[LuaError] Couldn't get lua_shared.dll factory. Scanning and detouring failed.\n" );
		}

		LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
		if( LUA->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
		{
			LUA->GetField( -1, "SERVER" );
			LUA->Remove( -2 ); // Remove global table from stack
			if( LUA->IsType( -1, GarrysMod::Lua::Type::BOOL ) && LUA->GetBool( -1 ) )
			{
				CSimpleScan server_scan;
				if( server_scan.SetDLL( "server.dll" ) )
				{
					if( !server_scan.FindFunction( HandleClientLuaError_signature, HandleClientLuaError_mask, &(void *&)HandleClientLuaError ) )
					{
						Msg( "[LuaError] Unable to detour function HandleClientLuaError.\n" );
					}

					if( !server_scan.FindFunction( Push_Entity_signature, Push_Entity_mask, &(void *&)Push_Entity ) )
					{
						Msg( "[LuaError] Unable to detour function Push_Entity.\n" );
					}
				}
				else
				{
					Msg( "[LuaError] Couldn't get server.dll factory. Scanning and detouring failed.\n" );
				}
			}

			LUA->Pop( 1 );
		}

		if( lua_shared != 0 )
		{
			FreeLibrary( lua_shared );
			lua_shared = 0;
		}
#else
#define garrysmod_bin_path "garrysmod/bin/" // This combined with lua_shared_file should ALWAYS work
											// unless some numbnut likes to change folder names for fun
#if __linux
#define lua_shared_file "lua_shared_srv.so"
#define server_file "server_srv.so"
#define lua_getstack_name "lua_getstack"
#define lua_getinfo_name "lua_getinfo"
#define luaL_loadbufferx_name "luaL_loadbufferx"
#define AdvancedLuaErrorReporter_name "_Z24AdvancedLuaErrorReporterP9lua_State"
#define HandleClientLuaError_name "_Z20HandleClientLuaErrorP11CBasePlayerPKc"
#define Push_Entity_name "_Z11Push_EntityP11CBaseEntity"
#elif __APPLE__
#define lua_shared_file garrysmod_bin_path "lua_shared.dylib"
#define server_file "server.dylib"
#define lua_getstack_name "_lua_getstack"
#define lua_getinfo_name "_lua_getinfo"
#define luaL_loadbufferx_name "_luaL_loadbufferx"
#define AdvancedLuaErrorReporter_name "__Z24AdvancedLuaErrorReporterP9lua_State"
#define HandleClientLuaError_name "__Z20HandleClientLuaErrorP11CBasePlayerPKc"
#define Push_Entity_name "__Z11Push_EntityP11CBaseEntity"

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

		void *lua_shared = dlopen( garrysmod_bin_path lua_shared_file, RTLD_NOW | RTLD_LOCAL );
		if( lua_shared != 0 )
		{
			if( ( lua_getstack = (lua_getstack_t)dlsym( lua_shared, lua_getstack_name ) ) == 0 )
			{
				Msg( "[LuaError] Unable to scan function lua_getstack.\n" );
			}

			if( ( lua_getinfo = (lua_getinfo_t)dlsym( lua_shared, lua_getinfo_name ) ) == 0 )
			{
				Msg( "[LuaError] Unable to scan function lua_getinfo.\n" );
			}

			if( ( luaL_loadbufferx = (luaL_loadbufferx_t)dlsym( lua_shared, luaL_loadbufferx_name ) ) == 0 )
			{
				Msg( "[LuaError] Unable to detour function luaL_loadbufferx.\n" );
			}

			if( ( AdvancedLuaErrorReporter = (AdvancedLuaErrorReporter_t)FindFunctionsTheHardWay( lua_shared, AdvancedLuaErrorReporter_name ) ) == 0 )
			{
				Msg( "[LuaError] Unable to detour function AdvancedLuaErrorReporter.\n" );
			}
		}
		else
		{
			Msg( "[LuaError] Couldn't open " lua_shared_file " file. Scanning and detouring failed.\n" );
		}

		LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
		if( LUA->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
		{
			LUA->GetField( -1, "SERVER" );
			LUA->Remove( -2 ); // Remove global table from stack
			if( LUA->IsType( -1, GarrysMod::Lua::Type::BOOL ) && LUA->GetBool( -1 ) )
			{
				void *server = dlopen( garrysmod_bin_path server_file, RTLD_NOW | RTLD_LOCAL );
				if( server != 0 )
				{
					if( ( HandleClientLuaError = (HandleClientLuaError_t)FindFunctionsTheHardWay( server, HandleClientLuaError_name ) ) == 0 )
					{
						Msg( "[LuaError] Unable to scan function HandleClientLuaError.\n" );
					}

					if( ( Push_Entity = (Push_Entity_t)FindFunctionsTheHardWay( server, Push_Entity_name ) ) == 0 )
					{
						Msg( "[LuaError] Unable to scan function Push_Entity.\n" );
					}
				}
				else
				{
					Msg( "[LuaError] Couldn't open " server_file " file. Scanning and detouring failed.\n" );
				}

				if( server != 0 )
				{
					dlclose( server );
					server = 0;
				}
			}

			LUA->Pop( 1 );
		}

		if( lua_shared != 0 )
		{
			dlclose( lua_shared );
			lua_shared = 0;
		}
#endif

		if( luaL_loadbufferx != 0 )
		{
			luaL_loadbufferx_detour = new MologieDetours::Detour<luaL_loadbufferx_t>( luaL_loadbufferx, luaL_loadbufferx_d );
		}

		if( AdvancedLuaErrorReporter != 0 )
		{
			AdvancedLuaErrorReporter_detour = new MologieDetours::Detour<AdvancedLuaErrorReporter_t>( AdvancedLuaErrorReporter, AdvancedLuaErrorReporter_d );
		}

		if( HandleClientLuaError != 0 )
		{
			HandleClientLuaError_detour = new MologieDetours::Detour<HandleClientLuaError_t>( HandleClientLuaError, HandleClientLuaError_d );
		}
	}

	if( !luaList.HasElement( LUA ) )
	{
		luaList.AddToTail( LUA );
	}

	return 0;
}

GMOD_MODULE_CLOSE( )
{
	luaList.FindAndRemove( LUA );
	if( luaList.Count( ) == 0 )
	{
		if( luaL_loadbufferx_detour != 0 )
		{
			delete luaL_loadbufferx_detour;
			luaL_loadbufferx_detour = 0;
		}

		if( AdvancedLuaErrorReporter_detour != 0 )
		{
			delete AdvancedLuaErrorReporter_detour;
			AdvancedLuaErrorReporter_detour = 0;
		}

		if( HandleClientLuaError_detour != 0 )
		{
			delete HandleClientLuaError_detour;
			HandleClientLuaError_detour = 0;
		}
	}

	return 0;
}
