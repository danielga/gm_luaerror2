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

#define CDECL __cdecl
#define FASTCALL __fastcall
#define THISCALL __thiscall

#define snprintf _snprintf

#if defined LUAERROR_SERVER

#define MAIN_BINARY_FILE "server.dll"

#elif defined LUAERROR_CLIENT

#define MAIN_BINARY_FILE "client.dll"

#endif

#define LUA_SHARED_BINARY "lua_shared.dll"

#elif defined __linux

#define CDECL __attribute__((cdecl))

#if defined LUAERROR_SERVER

#define MAIN_BINARY_FILE "garrysmod/bin/server_srv.so"
#define LUA_SHARED_BINARY "garrysmod/bin/lua_shared_srv.so"

#elif defined LUAERROR_CLIENT

#define MAIN_BINARY_FILE "garrysmod/bin/client.so"
#define LUA_SHARED_BINARY "garrysmod/bin/lua_shared.so"

#endif

#define SYMBOL_PREFIX "@"

#elif defined __APPLE__

#define CDECL __attribute__((cdecl))

#if defined LUAERROR_SERVER

#define MAIN_BINARY_FILE "garrysmod/bin/server.dylib"

#elif defined LUAERROR_CLIENT

#define MAIN_BINARY_FILE "garrysmod/bin/client.dylib"

#endif

#define LUA_SHARED_BINARY "garrysmod/bin/lua_shared.dylib"

#define SYMBOL_PREFIX "@_"

#endif

class CBaseEntity;
class CBasePlayer;

class CLuaGameCallback;
class CLuaError;

static GarrysMod::Lua::ILuaInterface *lua = NULL;

#if defined LUAERROR_SERVER

#if defined _WIN32

#define PUSH_ENTITY_SYM reinterpret_cast<const uint8_t *>( "\x55\x8B\xEC\x83\xEC\x14\x83\x3D\x2A\x2A\x2A\x2A\x2A\x74\x2A\x8B" )
#define PUSH_ENTITY_SYMLEN 16

#define HANDLECLIENTLUAERROR_SYM reinterpret_cast<const uint8_t *>( "\x55\x8B\xEC\x83\xEC\x08\x8B\x0D\x2A\x2A\x2A\x2A\x8B\x11\x53\x56" )
#define HANDLECLIENTLUAERROR_SYMLEN 16

#elif defined __linux || defined __APPLE__

#define PUSH_ENTITY_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "_Z11Push_EntityP11CBaseEntity" )
#define PUSH_ENTITY_SYMLEN 0

#define HANDLECLIENTLUAERROR_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "_Z20HandleClientLuaErrorP11CBasePlayerPKc" )
#define HANDLECLIENTLUAERROR_SYMLEN 0

#endif

typedef void ( *Push_Entity_t ) ( CBaseEntity *entity );

static Push_Entity_t Push_Entity = NULL;

typedef void ( CDECL *HandleClientLuaError_t ) ( CBasePlayer *player, const char *error );

static MologieDetours::Detour<HandleClientLuaError_t> *HandleClientLuaError_d = NULL;
static HandleClientLuaError_t HandleClientLuaError = NULL;

static void CDECL HandleClientLuaError_h( CBasePlayer *player, const char *error )
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
					return HandleClientLuaError( player, error );
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 3 );
					return;
				}

				lua->Pop( 1 );
			}
			else
			{
				lua->Pop( 1 );
			}
		}

		lua->Pop( 1 );
	}

	lua->Pop( 1 );
	return HandleClientLuaError( player, error );
}

#endif

#if defined _WIN32

#define CLUAGAMECALLBACK__LUAERROR_SYM reinterpret_cast<const uint8_t *>( "\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\x53\x56\x57\x33\xDB\x53\x89" )
#define CLUAGAMECALLBACK__LUAERROR_SYMLEN 16

#define LJ_ERR_LEX_SYM reinterpret_cast<const uint8_t *>( "\x81\xEC\x2A\x2A\x2A\x2A\x8B\x84\x24\x2A\x2A\x2A\x2A\x8B\x8C\x24" )
#define LJ_ERR_LEX_SYMLEN 16

#define LJ_ERR_RUN_SYM reinterpret_cast<const uint8_t *>( "\x56\x57\x8B\x7C\x24\x0C\x8B\xCF\xE8\x2A\x2A\x2A\x2A\x85\xC0\x74" )
#define LJ_ERR_RUN_SYMLEN 16

typedef void ( THISCALL *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );

#elif defined __linux || defined __APPLE__

#define CLUAGAMECALLBACK__LUAERROR_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "_ZN16CLuaGameCallback8LuaErrorEP9CLuaError" )
#define CLUAGAMECALLBACK__LUAERROR_SYMLEN 0

#define LJ_ERR_LEX_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "lj_err_lex" )
#define LJ_ERR_LEX_SYMLEN 0

#define LJ_ERR_RUN_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "lj_err_run" )
#define LJ_ERR_RUN_SYMLEN 0

typedef void ( CDECL *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );

#endif

typedef void ( CDECL *lj_err_lex_t )( lua_State *state, void *src, const char *tok, int32_t line, int em, va_list argp );
typedef void ( CDECL *lj_err_run_t )( lua_State *state );

static MologieDetours::Detour<CLuaGameCallback__LuaError_t> *CLuaGameCallback__LuaError_d = NULL;
static CLuaGameCallback__LuaError_t CLuaGameCallback__LuaError = NULL;

static MologieDetours::Detour<lj_err_lex_t> *lj_err_lex_d = NULL;
static lj_err_lex_t lj_err_lex = NULL;

static MologieDetours::Detour<lj_err_run_t> *lj_err_run_d = NULL;
static lj_err_run_t lj_err_run = NULL;

struct LuaDebug
{
	LuaDebug( const lua_Debug &debug ) :
		event( debug.event ),
		name( debug.name != NULL ? debug.name : "" ),
		namewhat( debug.namewhat != NULL ? debug.namewhat : "" ),
		what( debug.what != NULL ? debug.what : "" ),
		source( debug.source != NULL ? debug.source : "" ),
		currentline( debug.currentline ),
		nups( debug.nups ),
		linedefined( debug.linedefined ),
		lastlinedefined( debug.lastlinedefined ),
		short_src( debug.short_src ),
		i_ci( debug.i_ci )
	{ }

	int event;
	std::string name;
	std::string namewhat;
	std::string what;
	std::string source;
	int currentline;
	int nups;
	int linedefined;
	int lastlinedefined;
	std::string short_src;
	int i_ci;
};

static struct LuaErrorChain
{
	LuaErrorChain( ) :
		runtime( false ),
		source_line( -1 )
	{ }

	void Clear( )
	{
		runtime = false;
		source_file.clear( );
		source_line = -1;
		error_string.clear( );
		stack_data.clear( );
	}

	bool runtime;
	std::string source_file;
	int source_line;
	std::string error_string;
	std::vector<LuaDebug> stack_data;
} lua_error_chain;

#if defined _WIN32

static void FASTCALL CLuaGameCallback__LuaError_h( CLuaGameCallback *callback, void *, CLuaError *error )

#elif defined __linux || defined __APPLE__

static void CDECL CLuaGameCallback__LuaError_h( CLuaGameCallback *callback, CLuaError *error )

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
								LuaDebug &stacklevel = lua_error_chain.stack_data[i];

								lua->PushNumber( stacklevel.event );
								lua->SetField( -2, "event" );

								lua->PushString( stacklevel.name.c_str( ) );
								lua->SetField( -2, "name" );

								lua->PushString( stacklevel.namewhat.c_str( ) );
								lua->SetField( -2, "namewhat" );

								lua->PushString( stacklevel.what.c_str( ) );
								lua->SetField( -2, "what" );

								lua->PushString( stacklevel.source.c_str( ) );
								lua->SetField( -2, "source" );

								lua->PushNumber( stacklevel.currentline );
								lua->SetField( -2, "currentline" );

								lua->PushNumber( stacklevel.nups );
								lua->SetField( -2, "nups" );

								lua->PushNumber( stacklevel.linedefined );
								lua->SetField( -2, "linedefined" );

								lua->PushNumber( stacklevel.lastlinedefined );
								lua->SetField( -2, "lastlinedefined" );

								lua->PushString( stacklevel.short_src.c_str( ) );
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
						return CLuaGameCallback__LuaError( callback, error );
					}
				}
				else if( lua->PCall( 5, 1, 0 ) != 0 )
				{
					lua->Msg( "[LuaError hook error] %s\n", lua->GetString( ) );
					lua->Pop( 3 );
					return CLuaGameCallback__LuaError( callback, error );
				}

				if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) && lua->GetBool( ) )
				{
					lua->Pop( 3 );
					return;
				}

				lua->Pop( 1 );
			}
			else
			{
				lua->Pop( 1 );
			}
		}

		lua->Pop( 1 );
	}

	lua->Pop( 1 );
	return CLuaGameCallback__LuaError( callback, error );
}

static const char *lj_err_allmsg =
#define ERRDEF( name, msg ) msg "\0"
#include <lj_errmsg.h>
;

typedef enum
{
#define ERRDEF( name, msg ) LJ_ERR_##name, LJ_ERR_##name##_ = LJ_ERR_##name + sizeof( msg ) - 1,
#include <lj_errmsg.h>
	LJ_ERR__MAX
} ErrMsg;

#define err2msg( em ) ( lj_err_allmsg + ( em ) )

static void CDECL lj_err_lex_h( lua_State *state, void *src, const char *tok, int32_t line, int em, va_list argp )
{
	lua_error_chain.Clear( );

	lua_error_chain.runtime = false;

	const char *srcstr = reinterpret_cast<const char *>( reinterpret_cast<uintptr_t>( src ) + 16 );
	if( *srcstr == '@' )
		++srcstr;

	lua_error_chain.source_file = srcstr;
	lua_error_chain.source_line = line;

	int size = 256;
	lua_error_chain.error_string.resize( size );
	if( tok != NULL )
	{
		char temp[256] = { 0 };
		vsnprintf( temp, sizeof( temp ), err2msg( em ), argp );
		size = snprintf( &lua_error_chain.error_string[0], size, err2msg( LJ_ERR_XNEAR ), temp, tok );
	}
	else
	{
		size = vsnprintf( &lua_error_chain.error_string[0], size, err2msg( em ), argp );
	}

	lua_error_chain.error_string.resize( size );

	GarrysMod::Lua::ILuaInterface *lua_interface = reinterpret_cast<GarrysMod::Lua::ILuaInterface *>( LUA );
	lua_Debug dbg = { 0 };
	for( int level = 0; lua_interface->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		lua_interface->GetInfo( "Slnu", &dbg );
		lua_error_chain.stack_data.push_back( dbg );
	}

	return lj_err_lex( state, src, tok, line, em, argp );
}

static void CDECL lj_err_run_h( lua_State *state )
{
	lua_error_chain.Clear( );

	lua_error_chain.runtime = true;

	int top = LUA->Top( );
	const char *error = LUA->GetString( top > 1 ? top - 1 : 1 );
	if( error != NULL )
		lua_error_chain.error_string = error;

	GarrysMod::Lua::ILuaInterface *lua_interface = reinterpret_cast<GarrysMod::Lua::ILuaInterface *>( LUA );
	lua_Debug dbg = { 0 };
	for( int level = 0; lua_interface->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		lua_interface->GetInfo( "Slnu", &dbg );
		lua_error_chain.stack_data.push_back( dbg );
	}

	if( top > 0 )
	{
		const char *src = LUA->GetString( top );
		if( src != NULL )
		{
			lua_error_chain.source_file = src;
			size_t pos1 = lua_error_chain.source_file.find( ':' );
			size_t pos2 = lua_error_chain.source_file.find( ':', pos1 + 1 );
			if( pos1 != lua_error_chain.source_file.npos && pos2 != lua_error_chain.source_file.npos )
			{
				const char *linestart = &lua_error_chain.source_file[pos1 + 1];	
				const char *lineend = &lua_error_chain.source_file[pos2];
				char *linecheck = NULL;
				int line = strtol( linestart, &linecheck, 10 );
				if( linecheck == lineend )
				{
					lua_error_chain.source_line = line;
					lua_error_chain.source_file.resize( pos1 );
					return lj_err_run( state );
				}
			}
		}
	}

	if( lua_error_chain.stack_data.size( ) > 0 )
	{
		LuaDebug &debug = lua_error_chain.stack_data[0];
		lua_error_chain.source_file = debug.short_src;
		lua_error_chain.source_line = debug.currentline;
	}
	else
	{
		lua_error_chain.source_file = "[C]";
		lua_error_chain.source_line = -1;
	}

	return lj_err_run( state );
}

#define LUA_ERROR( error ) return _LuaError( state, error );
int _LuaError( lua_State *state, const char *error )
{
	char temp_error[300] = { 0 };
	snprintf( temp_error, sizeof( temp_error ), "Failed to load LuaError. '%s' Contact me in Facepunch (danielga) or Steam (tuestu1) with this error.", error );
	LUA->ThrowError( temp_error );
	return 0;
}

GMOD_MODULE_OPEN( )
{
	lua = reinterpret_cast<GarrysMod::Lua::ILuaInterface *>( LUA );

	SymbolFinder symfinder;

#if defined LUAERROR_SERVER

	HandleClientLuaError = reinterpret_cast<HandleClientLuaError_t>( symfinder.ResolveOnBinary( MAIN_BINARY_FILE, HANDLECLIENTLUAERROR_SYM, HANDLECLIENTLUAERROR_SYMLEN ) );
	if( HandleClientLuaError == NULL )
		LUA_ERROR( "Unable to sigscan function HandleClientLuaError (" MAIN_BINARY_FILE ")." );

	Push_Entity = reinterpret_cast<Push_Entity_t>( symfinder.ResolveOnBinary( MAIN_BINARY_FILE, PUSH_ENTITY_SYM, PUSH_ENTITY_SYMLEN ) );
	if( Push_Entity == NULL )
		LUA_ERROR( "Unable to sigscan function Push_Entity (" MAIN_BINARY_FILE ")." );

#endif

	CLuaGameCallback__LuaError = reinterpret_cast<CLuaGameCallback__LuaError_t>( symfinder.ResolveOnBinary( MAIN_BINARY_FILE, CLUAGAMECALLBACK__LUAERROR_SYM, CLUAGAMECALLBACK__LUAERROR_SYMLEN ) );
	if( CLuaGameCallback__LuaError == NULL )
		LUA_ERROR( "Unable to sigscan function CLuaGameCallback::LuaError (" MAIN_BINARY_FILE ")." );

	lj_err_lex = reinterpret_cast<lj_err_lex_t>( symfinder.FindPattern( LUA_SHARED_BINARY, LJ_ERR_LEX_SYM, LJ_ERR_LEX_SYMLEN ) );
	if( lj_err_lex == NULL )
		LUA_ERROR( "Unable to sigscan function lj_err_lex (" LUA_SHARED_BINARY ")." );

	lj_err_run = reinterpret_cast<lj_err_run_t>( symfinder.FindPattern( LUA_SHARED_BINARY, LJ_ERR_RUN_SYM, LJ_ERR_RUN_SYMLEN ) );
	if( lj_err_run == NULL )
		LUA_ERROR( "Unable to sigscan function lj_err_run (" LUA_SHARED_BINARY ")." );

	try
	{

#if defined LUAERROR_SERVER

		HandleClientLuaError_d = new MologieDetours::Detour<HandleClientLuaError_t>( HandleClientLuaError, HandleClientLuaError_h );
		HandleClientLuaError = HandleClientLuaError_d->GetOriginalFunction( );

#endif

		CLuaGameCallback__LuaError_d = new MologieDetours::Detour<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError, reinterpret_cast<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError_h ) );
		CLuaGameCallback__LuaError = CLuaGameCallback__LuaError_d->GetOriginalFunction( );

		lj_err_lex_d = new MologieDetours::Detour<lj_err_lex_t>( lj_err_lex, lj_err_lex_h );
		lj_err_lex = lj_err_lex_d->GetOriginalFunction( );

		lj_err_run_d = new MologieDetours::Detour<lj_err_run_t>( lj_err_run, lj_err_run_h );
		lj_err_run = lj_err_run_d->GetOriginalFunction( );

		lua->Msg( "[LuaError] Successfully loaded. Created by Daniel.\n" );
		return 0;

	}
	catch( std::exception &e )
	{
		LUA->PushString( e.what( ) );
	}

#if defined LUAERROR_SERVER

	if( HandleClientLuaError_d != NULL )
		delete HandleClientLuaError_d;

#endif

	if( CLuaGameCallback__LuaError_d != NULL )
		delete CLuaGameCallback__LuaError_d;

	if( lj_err_lex_d != NULL )
		delete lj_err_lex_d;

	if( lj_err_run_d != NULL )
		delete lj_err_run_d;

	LUA_ERROR( LUA->GetString( ) );
}

GMOD_MODULE_CLOSE( )
{

#if defined LUAERROR_SERVER

	delete HandleClientLuaError_d;

#endif

	delete CLuaGameCallback__LuaError_d;

	delete lj_err_lex_d;

	delete lj_err_run_d;

	lua->Msg( "[LuaError] Successfully unloaded. Thank you for using LuaError. Created by Daniel.\n" );
	return 0;
}
