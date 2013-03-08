#include <GarrysMod/Lua/Interface.h>
#include <string>
#include <utlvector.h>
#include <Color.h>
#include "MologieDetours/detours.h"
#include "ILuaInterface.h"

#if _WIN32
//Commented as it isn't needed
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include "csimplescan.h"
#define VOFFSET 0
#elif __linux
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <interface.h>
#define VOFFSET 1
#elif __APPLE__
#include <mach/task.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <interface.h>
#define VOFFSET 1
#endif

class CBaseEntity;
class CBasePlayer;
class CLuaGameCallback;

struct CLuaError
{
	const char *data;
};

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

ILuaInterface *lua = 0;

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

#if LUAERROR_SERVER
#if _WIN32
#define Push_Entity_signature "\x55\x8b\xec\x83\xec\x14\x83\x3d\x00\x00\x00\x00\x00\x74\x00\x8b\x4d\x08"
#define Push_Entity_mask "xxxxxxxx?????x?xxx"
#endif
typedef void ( *Push_Entity_t ) ( CBaseEntity *entity );
Push_Entity_t Push_Entity;

#if _WIN32
#define HandleClientLuaError_signature "\x55\x8b\xec\x83\xec\x08\x8b\x0d\x00\x00\x00\x00\x8b\x11\x53\x56"
#define HandleClientLuaError_mask "xxxxxxxx????xxxx"
#endif
typedef void ( __cdecl *HandleClientLuaError_t ) ( CBasePlayer *player, const char *error );
MologieDetours::Detour<HandleClientLuaError_t> *HandleClientLuaError_detour = 0;
HandleClientLuaError_t HandleClientLuaError = 0;
void __cdecl HandleClientLuaError_d( CBasePlayer *player, const char *error )
{
	lua->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
	{
		lua->GetField( -1, "hook" );
		if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
		{
			lua->GetField( -1, "Run" );
			if( lua->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
			{
				lua->PushString( "ClientLuaError" );
				Push_Entity( (CBaseEntity *)player );
				lua->PushString( error );

				if( lua->PCall( 3, 1, 0 ) != 0 )
				{
					ConColorMsg( Color( 255, 0, 0, 255 ), "[ClientLuaError hook error] %s\n", lua->GetString( ) );
					lua->Pop( 3 );
					return;
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 3 );
					return;
				}

				lua->Pop( 1 );
			}
		}

		lua->Pop( 1 );
	}

	lua->Pop( 1 );
	return HandleClientLuaError_detour->GetOriginalFunction( )( player, error );
}
#endif

#if _WIN32
#define CLuaGameCallback__LuaError_signature "\x55\x8b\xec\x81\xec\x00\x00\x00\x00\x53\x56\x57\x33\xdb\x53\x89"
#define CLuaGameCallback__LuaError_mask "xxxxx????xxxxxxx"
#endif
#if _WIN32
typedef void ( __thiscall *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );
#else
typedef void ( __cdecl *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );
#endif
MologieDetours::Detour<CLuaGameCallback__LuaError_t> *CLuaGameCallback__LuaError_detour = 0;
CLuaGameCallback__LuaError_t CLuaGameCallback__LuaError;
#if _WIN32
void __fastcall CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, void *fuckthis, CLuaError *error )
#else
void __cdecl CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, CLuaError *error )
#endif
{
	lua_State *state = lua->GetLuaState( );
	//const char *strerr = (const char *)( ( *(unsigned int *)error ) + VOFFSET * 4 );
	const char *strerr = error->data;
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

	int stacksize = stack.Count( );
	lua->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );
	if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
	{
		lua->GetField( -1, "hook" );
		if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
		{
			lua->GetField( -1, "Run" );
			if( lua->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
			{
				lua->PushString( "LuaError" );
				lua->PushBool( lua->IsServer( ) );
				lua->PushString( strerr );

				if( stacksize > 0 )
				{
					lua->CreateTable( );
					if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
					{
						for( int i = 0; i < stacksize; ++i )
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

					if( lua->PCall( 4, 1, 0 ) != 0 )
					{
						ConColorMsg( Color( 255, 0, 0, 255 ), "[LuaError hook error] %s\n", lua->GetString( ) );
						lua->Pop( 3 );
						return ( CLuaGameCallback__LuaError_detour->GetOriginalFunction( ) )( callback, error );
					}
				}
				else
				{
					if( lua->PCall( 3, 1, 0 ) != 0 )
					{
						ConColorMsg( Color( 255, 0, 0, 255 ), "[LuaError hook error] %s\n", lua->GetString( ) );
						lua->Pop( 3 );
						return ( CLuaGameCallback__LuaError_detour->GetOriginalFunction( ) )( callback, error );
					}
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 3 );
					return;
				}

				lua->Pop( 1 );
			}
		}

		lua->Pop( 1 );
	}

	lua->Pop( 1 );
	return ( CLuaGameCallback__LuaError_detour->GetOriginalFunction( ) )( callback, error );
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

#if _WIN32
#define snprintf _snprintf
#endif

#define LuaError( error ) _LuaError( state, error ); return 0;
int _LuaError( lua_State *state, const char *error )
{
	static char temp_error[300];
	snprintf( temp_error, sizeof( temp_error ), "Failed to load LuaError. '%s' Contact me in Facepunch (danielga) or Steam (tuestu1) with this error.", error );
	LUA->ThrowError( temp_error );
	return 0;
}

GMOD_MODULE_OPEN( )
{
	lua = (ILuaInterface *)LUA;

#if _WIN32
	HMODULE lua_shared = 0;
	//CSimpleScan lua_shared_scan;
	if( GetModuleHandleEx( 0, "lua_shared.dll", &lua_shared ) == TRUE )
	{
		/*
		if( !lua_shared_scan.FindFunction( lua_getstack_signature, lua_getstack_mask, (void **)&lua_getstack ) )
		{
			LuaError( "Unable to find function lua_getstack." );
		}

		if( !lua_shared_scan.FindFunction( lua_getinfo_signature, lua_getinfo_mask, (void **)&lua_getinfo ) )
		{
			LuaError( "Unable to find function lua_getinfo." );
		}
		*/

		if( ( lua_getstack = (lua_getstack_t)GetProcAddress( lua_shared, "lua_getstack" ) ) == 0 )
		{
			FreeLibrary( lua_shared );
			lua_shared = 0;
			LuaError( "Unable to find function lua_getstack." );
		}

		if( ( lua_getinfo = (lua_getinfo_t)GetProcAddress( lua_shared, "lua_getinfo" ) ) == 0 )
		{
			FreeLibrary( lua_shared );
			lua_shared = 0;
			LuaError( "Unable to find function lua_getinfo." );
		}
	}
	else
	{
		LuaError( "Couldn't open lua_shared.dll." );
	}

	if( lua_shared != 0 )
	{
		FreeLibrary( lua_shared );
		lua_shared = 0;
	}

#if LUAERROR_SERVER
	CSimpleScan server_scan;
	if( server_scan.SetDLL( "server.dll" ) )
	{
		if( !server_scan.FindFunction( HandleClientLuaError_signature, HandleClientLuaError_mask, (void **)&HandleClientLuaError ) )
		{
			LuaError( "Unable to detour function HandleClientLuaError." );
		}

		if( !server_scan.FindFunction( Push_Entity_signature, Push_Entity_mask, (void **)&Push_Entity ) )
		{
			LuaError( "Unable to find function Push_Entity." );
		}

		if( !server_scan.FindFunction( CLuaGameCallback__LuaError_signature, CLuaGameCallback__LuaError_mask, (void **)&CLuaGameCallback__LuaError ) )
		{
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (server.dll)." );
		}
	}
	else
	{
		LuaError( "Couldn't open server.dll." );
	}
#elif LUAERROR_CLIENT
	CSimpleScan client_scan;
	if( client_scan.SetDLL( "client.dll" ) )
	{
		if( !client_scan.FindFunction( CLuaGameCallback__LuaError_signature, CLuaGameCallback__LuaError_mask, (void **)&CLuaGameCallback__LuaError ) )
		{
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (client.dll)." );
		}
	}
	else
	{
		LuaError( "Couldn't open client.dll." );
	}
#endif
#else
#define garrysmod_bin_path "garrysmod/bin/" // This combined with lua_shared_file should ALWAYS work
											// unless some numbnut likes to change folder names for fun
#if __linux
#define lua_shared_file "lua_shared_srv.so"
#define server_file "server_srv.so"
#define client_file "client_srv.so"
#define FUNC_NAME_PREFIX ""
#elif __APPLE__
#define lua_shared_file "lua_shared.dylib"
#define server_file "server.dylib"
#define client_file "client.dylib"
#define FUNC_NAME_PREFIX "_"

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
#define lua_getstack_name FUNC_NAME_PREFIX "lua_getstack"
#define lua_getinfo_name FUNC_NAME_PREFIX "lua_getinfo"
#define HandleClientLuaError_name FUNC_NAME_PREFIX "_Z20HandleClientLuaErrorP11CBasePlayerPKc"
#define Push_Entity_name FUNC_NAME_PREFIX "_Z11Push_EntityP11CBaseEntity"
#define CLuaGameCallback__LuaError_name FUNC_NAME_PREFIX "_ZN16CLuaGameCallback8LuaErrorEP9CLuaError"

	void *lua_shared = dlopen( garrysmod_bin_path lua_shared_file, RTLD_NOW | RTLD_LOCAL );
	if( lua_shared != 0 )
	{
		if( ( lua_getstack = (lua_getstack_t)dlsym( lua_shared, lua_getstack_name ) ) == 0 )
		{
			dlclose( lua_shared );
			lua_shared = 0;
			LuaError( "Unable to find function lua_getstack." );
		}

		if( ( lua_getinfo = (lua_getinfo_t)dlsym( lua_shared, lua_getinfo_name ) ) == 0 )
		{
			dlclose( lua_shared );
			lua_shared = 0;
			LuaError( "Unable to find function lua_getinfo." );
		}
	}
	else
	{
		LuaError( "Couldn't open " lua_shared_file " file." );
	}

	if( lua_shared != 0 )
	{
		dlclose( lua_shared );
		lua_shared = 0;
	}

#if LUAERROR_SERVER
	void *server = dlopen( garrysmod_bin_path server_file, RTLD_NOW | RTLD_LOCAL );
	if( server != 0 )
	{
		if( ( HandleClientLuaError = (HandleClientLuaError_t)FindFunctionsTheHardWay( server, HandleClientLuaError_name ) ) == 0 )
		{
			dlclose( server );
			server = 0;
			LuaError( "Unable to detour function HandleClientLuaError." );
		}

		if( ( Push_Entity = (Push_Entity_t)FindFunctionsTheHardWay( server, Push_Entity_name ) ) == 0 )
		{
			dlclose( server );
			server = 0;
			LuaError( "Unable to find function Push_Entity." );
		}

		if( ( CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)FindFunctionsTheHardWay( server, CLuaGameCallback__LuaError_name ) ) == 0 )
		{
			dlclose( server );
			server = 0;
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (" server_file ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " server_file " file." );
	}

	if( server != 0 )
	{
		dlclose( server );
		server = 0;
	}
#elif LUAERROR_CLIENT
	void *client = dlopen( garrysmod_bin_path client_file, RTLD_NOW | RTLD_LOCAL );
	if( client != 0 )
	{
		if( ( CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)FindFunctionsTheHardWay( client, CLuaGameCallback__LuaError_name ) ) == 0 )
		{
			dlclose( client );
			client = 0;
			LuaError( "Unable to detour function CLuaGameCallback__LuaError  (" client_file ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " client_file " file." );
	}

	if( client != 0 )
	{
		dlclose( client );
		client = 0;
	}
#endif
#endif

	ConColorMsg( Color( 0, 255, 0, 255 ), "[LuaError] Successfully loaded. Created by Daniel." );
	return 0;
}

GMOD_MODULE_CLOSE( )
{
#if LUAERROR_SERVER
	if( HandleClientLuaError_detour != 0 )
	{
		delete HandleClientLuaError_detour;
		HandleClientLuaError_detour = 0;
	}
#endif

	if( CLuaGameCallback__LuaError_detour != 0 )
	{
		delete CLuaGameCallback__LuaError_detour;
		CLuaGameCallback__LuaError_detour = 0;
	}

	ConColorMsg( Color( 0, 255, 0, 255 ), "[LuaError] Successfully unloaded. Thank you for using LuaError. Created by Daniel." );
	return 0;
}
