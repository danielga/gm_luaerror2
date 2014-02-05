#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaInterface.h>
#include <SymbolFinder.hpp>
#include <MologieDetours/detours.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>

#if defined _WIN32

#define snprintf _snprintf

#elif defined __linux || defined __APPLE__

#include <dlfcn.h>

#define __cdecl __attribute__((__cdecl__))
#define __fastcall __attribute__((__fastcall__))

#endif

class CBaseEntity;
class CBasePlayer;

class CLuaGameCallback;
class CLuaError;

GarrysMod::Lua::ILuaInterface *lua = NULL;

#if defined LUAERROR_SERVER

#if defined _WIN32

#define Push_Entity_sig "\x55\x8B\xEC\x83\xEC\x14\x83\x3D\x2A\x2A\x2A\x2A\x2A\x74\x2A\x8B"
#define Push_Entity_siglen 16

#define HandleClientLuaError_sig "\x55\x8B\xEC\x83\xEC\x08\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x11\x53\x56"
#define HandleClientLuaError_siglen 16

#endif

typedef void ( *Push_Entity_t ) ( CBaseEntity *entity );
Push_Entity_t Push_Entity = NULL;

typedef void ( __cdecl *HandleClientLuaError_t ) ( CBasePlayer *player, const char *error );
MologieDetours::Detour<HandleClientLuaError_t> *HandleClientLuaError_detour = NULL;
HandleClientLuaError_t HandleClientLuaError = NULL;

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
				Push_Entity( reinterpret_cast<CBaseEntity *>( player ) );
				lua->PushString( error );

				if( lua->PCall( 3, 1, 0 ) != 0 )
				{
					lua->Msg( "[ClientLuaError hook error] %s\n", lua->GetString( ) );
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

#if defined _WIN32

#define CLuaGameCallback__LuaError_sig "\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\x53\x56\x57\x33\xDB\x53\x89"
#define CLuaGameCallback__LuaError_siglen 16

#define lj_err_lex_sig "\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x2A\x2A\x2A\x2A\x8B\x8C\x24"
#define lj_err_lex_siglen 16

#define lj_err_run_sig "\x56\x57\x8B\x7C\x24\x0C\x8B\xCF\xE8\x2A\x2A\x2A\x2A\x85\xC0\x74"
#define lj_err_run_siglen 16

typedef void ( __thiscall *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );

#elif defined __linux || defined __APPLE__

typedef void ( __cdecl *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );

#endif

typedef void ( __cdecl *lj_err_lex_t )( lua_State *state, void *src, const char *tok, int32_t line, int em, va_list argp );
typedef void ( __cdecl *lj_err_run_t )( lua_State *state );

MologieDetours::Detour<CLuaGameCallback__LuaError_t> *CLuaGameCallback__LuaError_detour = NULL;
CLuaGameCallback__LuaError_t CLuaGameCallback__LuaError = NULL;

MologieDetours::Detour<lj_err_lex_t> *lj_err_lex_detour = NULL;
lj_err_lex_t lj_err_lex = NULL;

MologieDetours::Detour<lj_err_run_t> *lj_err_run_detour = NULL;
lj_err_run_t lj_err_run = NULL;

static struct LuaErrorChain
{
	LuaErrorChain( ) :
		inside_chain( false ),
		runtime( false ),
		source_line( -1 )
	{ }

	bool HasEntered( )
	{
		return inside_chain;
	}

	void Enter( )
	{
		inside_chain = true;
	}

	void Exit( )
	{
		inside_chain = false;
		runtime = false;
		source_file.clear( );
		source_line = -1;
		error_string.clear( );
		stack_data.clear( );
	}

	bool inside_chain;
	bool runtime;
	std::string source_file;
	int source_line;
	std::string error_string;
	std::vector<lua_Debug> stack_data;
} lua_error_chain;

#if defined _WIN32
void __fastcall CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, void *, CLuaError *error )
#elif defined __linux || defined __APPLE__
void __cdecl CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, CLuaError *error )
#endif
{
	size_t stacksize = lua_error_chain.stack_data.size( );
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

				lua->PushBool( lua_error_chain.runtime );

				lua->PushString( lua_error_chain.source_file.c_str( ) );
				lua->PushNumber( lua_error_chain.source_line );

				lua->PushString( lua_error_chain.error_string.c_str( ) );

				if( stacksize > 0 )
				{
					lua->CreateTable( );
					if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
					{
						for( size_t i = 0; i < stacksize; ++i )
						{
							lua->PushNumber( i + 1 );
							lua->CreateTable( );
							if( lua->IsType( -1, GarrysMod::Lua::Type::TABLE ) )
							{
								lua_Debug &stacklevel = lua_error_chain.stack_data[i];

								lua->PushNumber( stacklevel.event );
								lua->SetField( -2, "event" );

								lua->PushString( stacklevel.name );
								lua->SetField( -2, "name" );

								lua->PushString( stacklevel.namewhat );
								lua->SetField( -2, "namewhat" );

								lua->PushString( stacklevel.what );
								lua->SetField( -2, "what" );

								lua->PushString( stacklevel.source );
								lua->SetField( -2, "source" );

								lua->PushNumber( stacklevel.currentline );
								lua->SetField( -2, "currentline" );

								lua->PushNumber( stacklevel.nups );
								lua->SetField( -2, "nups" );

								lua->PushNumber( stacklevel.linedefined );
								lua->SetField( -2, "linedefined" );

								lua->PushNumber( stacklevel.lastlinedefined );
								lua->SetField( -2, "lastlinedefined" );

								lua->PushString( stacklevel.short_src );
								lua->SetField( -2, "short_src" );

								lua->PushNumber( stacklevel.i_ci );
								lua->SetField( -2, "i_ci" );
							}

							lua->SetTable( -3 );
						}
					}

					if( lua->PCall( 6, 1, 0 ) != 0 )
					{
						lua->Msg( "[LuaError hook error] %s\n", lua->GetString( ) );
						lua->Pop( 3 );

						lua_error_chain.Exit( );
						return CLuaGameCallback__LuaError_detour->GetOriginalFunction( )( callback, error );
					}
				}
				else if( lua->PCall( 5, 1, 0 ) != 0 )
				{
					lua->Msg( "[LuaError hook error] %s\n", lua->GetString( ) );
					lua->Pop( 3 );

					lua_error_chain.Exit( );
					return CLuaGameCallback__LuaError_detour->GetOriginalFunction( )( callback, error );
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 3 );

					lua_error_chain.Exit( );
					return;
				}

				lua->Pop( 1 );
			}
		}

		lua->Pop( 1 );
	}

	lua->Pop( 1 );

	lua_error_chain.Exit( );
	return CLuaGameCallback__LuaError_detour->GetOriginalFunction( )( callback, error );
}

const char *lj_err_allmsg =
#define ERRDEF( name, msg ) msg "\0"
#include <lj_errmsg.h>
;

typedef enum
{
#define ERRDEF( name, msg ) \
	LJ_ERR_##name, LJ_ERR_##name##_ = LJ_ERR_##name + sizeof( msg ) - 1,
#include <lj_errmsg.h>
	LJ_ERR__MAX
} ErrMsg;

#define err2msg( em ) ( lj_err_allmsg + ( em ) )

void __cdecl lj_err_lex_d( lua_State *state, void *src, const char *tok, int32_t line, int em, va_list argp )
{
	if( lua_error_chain.HasEntered( ) )
		return lj_err_lex_detour->GetOriginalFunction( )( state, src, tok, line, em, argp );

	lua_error_chain.Enter( );

	lua_error_chain.runtime = false;

	const char *srcstr = reinterpret_cast<const char *>( reinterpret_cast<uintptr_t>( src ) + 16 );
	if( *srcstr == '@' )
		++srcstr;

	lua_error_chain.source_file = srcstr;
	lua_error_chain.source_line = line;

	char strerr[512] = { 0 };
	if( tok != NULL )
	{
		vsnprintf( &strerr[sizeof( strerr ) / 2 - 1], sizeof( strerr ) / 2, err2msg( em ), argp );
		snprintf( strerr, sizeof( strerr ) / 2, err2msg( LJ_ERR_XNEAR ), &strerr[sizeof( strerr ) / 2 - 1], tok );
	}
	else
	{
		vsnprintf( strerr, sizeof( strerr ), err2msg( em ), argp );
	}

	lua_error_chain.error_string = strerr;

	GarrysMod::Lua::ILuaInterface *lua_interface = static_cast<GarrysMod::Lua::ILuaInterface *>( LUA );
	lua_Debug dbg = { 0 };
	for( int level = 0; lua_interface->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		lua_interface->GetInfo( "Slnu", &dbg );
		lua_error_chain.stack_data.push_back( dbg );
	}

	return lj_err_lex_detour->GetOriginalFunction( )( state, src, tok, line, em, argp );
}

void __cdecl lj_err_run_d( lua_State *state )
{
	if( lua_error_chain.HasEntered( ) )
		return lj_err_run_detour->GetOriginalFunction( )( state );

	lua_error_chain.Enter( );

	lua_error_chain.runtime = true;

	lua_error_chain.error_string = LUA->GetString( LUA->Top( ) - 1 );

	GarrysMod::Lua::ILuaInterface *lua_interface = static_cast<GarrysMod::Lua::ILuaInterface *>( LUA );
	lua_Debug dbg = { 0 };
	for( int level = 0; lua_interface->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		lua_interface->GetInfo( "Slnu", &dbg );
		lua_error_chain.stack_data.push_back( dbg );
	}

	lua_error_chain.source_file = LUA->GetString( LUA->Top( ) );
	size_t pos = lua_error_chain.source_file.find( ':' );
	lua_error_chain.source_line = atoi( &lua_error_chain.source_file[pos + 1] );
	lua_error_chain.source_file.resize( pos );

	return lj_err_run_detour->GetOriginalFunction( )( state );
}

#define LuaError( error ) return _LuaError( state, error );
int _LuaError( lua_State *state, const char *error )
{
	char temp_error[300] = { 0 };
	snprintf( temp_error, sizeof( temp_error ), "Failed to load LuaError. '%s' Contact me in Facepunch (danielga) or Steam (tuestu1) with this error.", error );
	LUA->ThrowError( temp_error );
	return 0;
}

GMOD_MODULE_OPEN( )
{
	lua = static_cast<GarrysMod::Lua::ILuaInterface *>( LUA );

	SymbolFinder symfinder;

#if defined _WIN32

#if defined LUAERROR_SERVER
#define BINARY_FILE "server.dll"
#elif defined LUAERROR_CLIENT
#define BINARY_FILE "client.dll"
#endif

#define LUA_SHARED_BINARY "lua_shared.dll"

	HMODULE binary = LoadLibrary( BINARY_FILE );
	if( binary != NULL )
	{
#if defined LUAERROR_SERVER

		HandleClientLuaError = static_cast<HandleClientLuaError_t>( symfinder.FindPattern( binary, HandleClientLuaError_sig, HandleClientLuaError_siglen ) );
		if( HandleClientLuaError == NULL )
		{
			FreeLibrary( binary );
			binary = NULL;
			LuaError( "Unable to sigscan function HandleClientLuaError (" BINARY_FILE ")." );
		}

		Push_Entity = static_cast<Push_Entity_t>( symfinder.FindPattern( binary, Push_Entity_sig, Push_Entity_siglen ) );
		if( Push_Entity == NULL )
		{
			FreeLibrary( binary );
			binary = NULL;
			LuaError( "Unable to sigscan function Push_Entity (" BINARY_FILE ")." );
		}

#endif

		CLuaGameCallback__LuaError = static_cast<CLuaGameCallback__LuaError_t>( symfinder.FindPattern( binary, CLuaGameCallback__LuaError_sig, CLuaGameCallback__LuaError_siglen ) );
		if( CLuaGameCallback__LuaError == NULL )
		{
			FreeLibrary( binary );
			binary = NULL;
			LuaError( "Unable to sigscan function CLuaGameCallback::LuaError (" BINARY_FILE ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " BINARY_FILE "." );
	}

	if( binary != NULL )
	{
		FreeLibrary( binary );
		binary = NULL;
	}

	HMODULE lua_shared = LoadLibrary( LUA_SHARED_BINARY );
	if( lua_shared != NULL )
	{
		lj_err_lex = static_cast<lj_err_lex_t>( symfinder.FindPattern( lua_shared, lj_err_lex_sig, lj_err_lex_siglen ) );
		if( lj_err_lex == NULL )
		{
			FreeLibrary( lua_shared );
			lua_shared = NULL;
			LuaError( "Unable to sigscan function lj_err_lex (" LUA_SHARED_BINARY ")." );
		}

		lj_err_run = static_cast<lj_err_run_t>( symfinder.FindPattern( lua_shared, lj_err_run_sig, lj_err_run_siglen ) );
		if( lj_err_run == NULL )
		{
			FreeLibrary( lua_shared );
			lua_shared = NULL;
			LuaError( "Unable to sigscan function lj_err_run (" LUA_SHARED_BINARY ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " LUA_SHARED_BINARY "." );
	}

	if( lua_shared != NULL )
	{
		FreeLibrary( lua_shared );
		lua_shared = NULL;
	}

#elif defined __linux || defined __APPLE

#define GARRYSMOD_BIN_PATH "garrysmod/bin/"

#if defined __linux

#if defined LUAERROR_SERVER
#define MAIN_BINARY_FILE "server_srv.so"
#define LUA_SHARED_BINARY "lua_shared_srv.so"
#elif defined LUAERROR_CLIENT
#define MAIN_BINARY_FILE "client.so"
#define LUA_SHARED_BINARY "lua_shared.so"
#endif

#define FUNC_NAME_PREFIX ""

#elif defined __APPLE__

#if defined LUAERROR_SERVER
#define MAIN_BINARY_FILE "server.dylib"
#elif defined LUAERROR_CLIENT
#define MAIN_BINARY_FILE "client.dylib"
#endif

#define LUA_SHARED_BINARY "lua_shared.dylib"

#define FUNC_NAME_PREFIX "_"

#endif

	void *binary = dlopen( GARRYSMOD_BIN_PATH MAIN_BINARY_FILE, RTLD_NOW | RTLD_LOCAL );
	if( binary != NULL )
	{
#if defined LUAERROR_SERVER

		HandleClientLuaError = reinterpret_cast<HandleClientLuaError_t>( symfinder.FindSymbol( binary, FUNC_NAME_PREFIX "_Z20HandleClientLuaErrorP11CBasePlayerPKc" ) );
		if( HandleClientLuaError == NULL )
		{
			dlclose( binary );
			binary = NULL;
			LuaError( "Unable to detour function HandleClientLuaError (" MAIN_BINARY_FILE ")." );
		}

		Push_Entity = reinterpret_cast<Push_Entity_t>( symfinder.FindSymbol( binary, FUNC_NAME_PREFIX "_Z11Push_EntityP11CBaseEntity" ) );
		if( Push_Entity == NULL )
		{
			dlclose( binary );
			binary = NULL;
			LuaError( "Unable to find function Push_Entity (" MAIN_BINARY_FILE ")." );
		}

#endif

		CLuaGameCallback__LuaError = reinterpret_cast<CLuaGameCallback__LuaError_t>( symfinder.FindSymbol( binary, FUNC_NAME_PREFIX "_ZN16CLuaGameCallback8LuaErrorEP9CLuaError" ) );
		if( CLuaGameCallback__LuaError == NULL )
		{
			dlclose( binary );
			binary = NULL;
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (" MAIN_BINARY_FILE ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " MAIN_BINARY_FILE " file." );
	}

	if( binary != NULL )
	{
		dlclose( binary );
		binary = NULL;
	}

	void *lua_shared = dlopen( GARRYSMOD_BIN_PATH LUA_SHARED_BINARY, RTLD_NOW | RTLD_LOCAL );
	if( lua_shared != NULL )
	{
		lj_err_lex = reinterpret_cast<lj_err_lex_t>( symfinder.FindSymbol( lua_shared, FUNC_NAME_PREFIX "lj_err_lex" ) );
		if( lj_err_lex == NULL )
		{
			dlclose( lua_shared );
			lua_shared = NULL;
			LuaError( "Unable to sigscan function lj_err_lex (" LUA_SHARED_BINARY ")." );
		}

		lj_err_run = reinterpret_cast<lj_err_run_t>( symfinder.FindSymbol( lua_shared, FUNC_NAME_PREFIX "lj_err_run" ) );
		if( lj_err_run == NULL )
		{
			dlclose( lua_shared );
			lua_shared = NULL;
			LuaError( "Unable to sigscan function lj_err_run (" LUA_SHARED_BINARY ")." );
		}
	}
	else
	{
		LuaError( "Couldn't open " LUA_SHARED_BINARY "." );
	}

	if( lua_shared != NULL )
	{
		dlclose( lua_shared );
		lua_shared = NULL;
	}

#endif

	try
	{

#if defined LUAERROR_SERVER

		HandleClientLuaError_detour = new MologieDetours::Detour<HandleClientLuaError_t>( HandleClientLuaError, HandleClientLuaError_d );

#endif

		CLuaGameCallback__LuaError_detour = new MologieDetours::Detour<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError, reinterpret_cast<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError_d ) );

		lj_err_lex_detour = new MologieDetours::Detour<lj_err_lex_t>( lj_err_lex, lj_err_lex_d );

		lj_err_run_detour = new MologieDetours::Detour<lj_err_run_t>( lj_err_run, lj_err_run_d );

	}
	catch( std::runtime_error &e )
	{
		printf( "%s\n", e.what( ) );
	}

	lua->Msg( "[LuaError] Successfully loaded. Created by Daniel.\n" );
	return 0;
}

GMOD_MODULE_CLOSE( )
{
#if defined LUAERROR_SERVER

	if( HandleClientLuaError_detour != NULL )
	{
		delete HandleClientLuaError_detour;
		HandleClientLuaError_detour = NULL;
	}

#endif

	if( CLuaGameCallback__LuaError_detour != NULL )
	{
		delete CLuaGameCallback__LuaError_detour;
		CLuaGameCallback__LuaError_detour = NULL;
	}

	if( lj_err_lex_detour != NULL )
	{
		delete lj_err_lex_detour;
		lj_err_lex_detour = NULL;
	}

	if( lj_err_run_detour != NULL )
	{
		delete lj_err_run_detour;
		lj_err_run_detour = NULL;
	}

	lua->Msg( "[LuaError] Successfully unloaded. Thank you for using LuaError. Created by Daniel.\n" );
	return 0;
}
