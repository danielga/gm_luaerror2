#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaInterface.h>
#include <SymbolFinder.hpp>
#include <MologieDetours/detours.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <sstream>
#include <vector>

#if defined _WIN32

#define __CDECL __cdecl
#define __FASTCALL __fastcall
#define __THISCALL __thiscall

#if defined LUAERROR_SERVER

#define MAIN_BINARY_FILE "server.dll"

#elif defined LUAERROR_CLIENT

#define MAIN_BINARY_FILE "client.dll"

#endif

#define LUA_SHARED_BINARY "lua_shared.dll"

#elif defined __linux

#define __CDECL __attribute__((cdecl))

#if defined LUAERROR_SERVER

#define MAIN_BINARY_FILE "garrysmod/bin/server_srv.so"
#define LUA_SHARED_BINARY "garrysmod/bin/lua_shared_srv.so"

#elif defined LUAERROR_CLIENT

#define MAIN_BINARY_FILE "garrysmod/bin/client.so"
#define LUA_SHARED_BINARY "garrysmod/bin/lua_shared.so"

#endif

#define SYMBOL_PREFIX "@"

#elif defined __APPLE__

#define __CDECL __attribute__((cdecl))

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

typedef void ( __CDECL *HandleClientLuaError_t ) ( CBasePlayer *player, const char *error );

static MologieDetours::Detour<HandleClientLuaError_t> *HandleClientLuaError_d = NULL;
static HandleClientLuaError_t HandleClientLuaError = NULL;

static int ClientLuaErrorHookCall( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

	LUA->GetField( -1, "hook" );

	LUA->GetField( -1, "Run" );

	LUA->PushString( "ClientLuaError" );

	LUA->Push( 1 );
	LUA->Push( 2 );

	LUA->Call( 3, 1 );
	return 1;
}

static void __CDECL HandleClientLuaError_h( CBasePlayer *player, const char *error )
{
	lua->PushCFunction( ClientLuaErrorHookCall );

	Push_Entity( reinterpret_cast<CBaseEntity *>( player ) );
	lua->PushString( error );

	bool call_original = true;
	if( lua->PCall( 2, 1, 0 ) != 0 )
		lua->Msg( "[ClientLuaError hook error] %s\n", lua->GetString( -1 ) );
	else if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) )
		call_original = !lua->GetBool( -1 );

	lua->Pop( 1 );

	if( call_original )
		HandleClientLuaError( player, error );
}

#endif

#if defined _WIN32

#define CLUAGAMECALLBACK__LUAERROR_SYM reinterpret_cast<const uint8_t *>( "\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\x53\x56\x57\x33\xDB\x53\x89" )
#define CLUAGAMECALLBACK__LUAERROR_SYMLEN 16

typedef void ( __THISCALL *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, std::string &error );

#elif defined __linux || defined __APPLE__

#define CLUAGAMECALLBACK__LUAERROR_SYM reinterpret_cast<const uint8_t *>( SYMBOL_PREFIX "_ZN16CLuaGameCallback8LuaErrorEP9CLuaError" )
#define CLUAGAMECALLBACK__LUAERROR_SYMLEN 0

typedef void ( __CDECL *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, std::string &error );

#endif

static MologieDetours::Detour<CLuaGameCallback__LuaError_t> *CLuaGameCallback__LuaError_d = NULL;
static CLuaGameCallback__LuaError_t CLuaGameCallback__LuaError = NULL;

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

static void ParseErrorString( const std::string &error )
{
	if( error.size( ) == 0 )
		return;

	std::istringstream strstream( error );

	std::getline( strstream, lua_error_chain.source_file, ':' );

	strstream >> lua_error_chain.source_line;

	strstream.ignore( 2 ); // remove : and <space>

	std::getline( strstream, lua_error_chain.error_string );
}

int reporter_ref = -1;

static int __CDECL AdvancedLuaErrorReporter( lua_State *state )
{
	lua_error_chain.runtime = true;

	ParseErrorString( LUA->GetString( 1 ) );

	GarrysMod::Lua::ILuaInterface *lua_interface = reinterpret_cast<GarrysMod::Lua::ILuaInterface *>( LUA );
	lua_Debug dbg = { 0 };
	for( int level = 0; lua_interface->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
	{
		lua_interface->GetInfo( "Slnu", &dbg );
		lua_error_chain.stack_data.push_back( dbg );
	}

	LUA->ReferencePush( reporter_ref );
	LUA->Push( 1 );
	LUA->Call( 1, 1 );
	return 1;
}

static int LuaErrorHookCall( lua_State *state )
{
	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_GLOB );

	LUA->GetField( -1, "hook" );

	LUA->GetField( -1, "Run" );

	LUA->PushString( "LuaError" );

	LUA->PushBool( lua_error_chain.runtime );
	LUA->PushString( lua_error_chain.source_file.c_str( ) );
	LUA->PushNumber( lua_error_chain.source_line );
	LUA->PushString( lua_error_chain.error_string.c_str( ) );

	int args = 5;
	size_t stacksize = lua_error_chain.stack_data.size( );
	if( stacksize > 0 )
	{
		args = 6;

		LUA->CreateTable( );
		for( size_t i = 0; i < stacksize; ++i )
		{
			LUA->PushNumber( i + 1 );
			LUA->CreateTable( );

			LuaDebug &stacklevel = lua_error_chain.stack_data[i];

			LUA->PushNumber( stacklevel.event );
			LUA->SetField( -2, "event" );

			LUA->PushString( stacklevel.name.c_str( ) );
			LUA->SetField( -2, "name" );

			LUA->PushString( stacklevel.namewhat.c_str( ) );
			LUA->SetField( -2, "namewhat" );

			LUA->PushString( stacklevel.what.c_str( ) );
			LUA->SetField( -2, "what" );

			LUA->PushString( stacklevel.source.c_str( ) );
			LUA->SetField( -2, "source" );

			LUA->PushNumber( stacklevel.currentline );
			LUA->SetField( -2, "currentline" );

			LUA->PushNumber( stacklevel.nups );
			LUA->SetField( -2, "nups" );

			LUA->PushNumber( stacklevel.linedefined );
			LUA->SetField( -2, "linedefined" );

			LUA->PushNumber( stacklevel.lastlinedefined );
			LUA->SetField( -2, "lastlinedefined" );

			LUA->PushString( stacklevel.short_src.c_str( ) );
			LUA->SetField( -2, "short_src" );

			LUA->PushNumber( stacklevel.i_ci );
			LUA->SetField( -2, "i_ci" );

			LUA->SetTable( -3 );
		}
	}

	LUA->Call( args, 1 );
	return 1;
}

#if defined _WIN32

static void __FASTCALL CLuaGameCallback__LuaError_h( CLuaGameCallback *callback, void *, std::string &error )

#elif defined __linux || defined __APPLE__

static void __CDECL CLuaGameCallback__LuaError_h( CLuaGameCallback *callback, std::string &error )

#endif

{
	if( !lua_error_chain.runtime )
	{
		ParseErrorString( error );

		lua_Debug dbg = { 0 };
		for( int level = 0; lua->GetStack( level, &dbg ) == 1; ++level, memset( &dbg, 0, sizeof( dbg ) ) )
		{
			lua->GetInfo( "Slnu", &dbg );
			lua_error_chain.stack_data.push_back( dbg );
		}
	}

	lua->PushCFunction( LuaErrorHookCall );

	bool call_original = true;
	if( lua->PCall( 0, 1, 0 ) != 0 )
		lua->Msg( "[LuaError hook error] %s\n", lua->GetString( -1 ) );
	else if( lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) )
		call_original = !lua->GetBool( -1 );

	lua->Pop( 1 );

	lua_error_chain.Clear( );

	if( call_original )
		CLuaGameCallback__LuaError_d->GetOriginalFunction( )( callback, error );
}

#define LUA_ERROR( error ) return _LuaError( state, error );
int _LuaError( lua_State *state, const char *error )
{
	char temp_error[300] = { 0 };

#if defined _WIN32

	_snprintf_s( temp_error, sizeof( temp_error ), "Failed to load LuaError. '%s' Contact me in Facepunch (danielga) or Steam (tuestu1) with this error.", error );

#else

	snprintf( temp_error, sizeof( temp_error ), "Failed to load LuaError. '%s' Contact me in Facepunch (danielga) or Steam (tuestu1) with this error.", error );

#endif

	LUA->ThrowError( temp_error );
	return 0;
}

GMOD_MODULE_OPEN( )
{
	lua = reinterpret_cast<GarrysMod::Lua::ILuaInterface *>( LUA );

	LUA->PushSpecial( GarrysMod::Lua::SPECIAL_REG );

	LUA->PushNumber( 1 );
	LUA->GetTable( -2 );
	if( LUA->IsType( -1, GarrysMod::Lua::Type::FUNCTION ) )
	{
		LUA->Pop( 1 );

		LUA->PushNumber( 1 );
		LUA->GetTable( -2 );
		reporter_ref = LUA->ReferenceCreate( );

		LUA->PushNumber( 1 );
		LUA->PushCFunction( AdvancedLuaErrorReporter );
		LUA->SetTable( -3 );

		LUA->Pop( 1 );
	}
	else
		LUA_ERROR( "Unable to detour AdvancedLuaErrorReporter." );

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

	try
	{

#if defined LUAERROR_SERVER

		HandleClientLuaError_d = new MologieDetours::Detour<HandleClientLuaError_t>( HandleClientLuaError, HandleClientLuaError_h );
		HandleClientLuaError = HandleClientLuaError_d->GetOriginalFunction( );

#endif

		CLuaGameCallback__LuaError_d = new MologieDetours::Detour<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError, reinterpret_cast<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError_h ) );
		CLuaGameCallback__LuaError = CLuaGameCallback__LuaError_d->GetOriginalFunction( );

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

	LUA_ERROR( LUA->GetString( ) );
}

GMOD_MODULE_CLOSE( )
{
	// classic C one-liner to remove "useless" warnings.
	(void)state;

#if defined LUAERROR_SERVER

	delete HandleClientLuaError_d;

#endif

	delete CLuaGameCallback__LuaError_d;

	lua->Msg( "[LuaError] Successfully unloaded. Thank you for using LuaError. Created by Daniel.\n" );
	return 0;
}
