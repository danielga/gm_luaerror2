#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaInterface.h>
#include "SymbolFinder.hpp"
#include "MologieDetours/detours.h"
#include <vector>

#ifdef _WIN32
#define VOFFSET 0
#elif defined __linux || defined __APPLE__
#define VOFFSET 1
#endif

class CBaseEntity;
class CBasePlayer;
class CLuaGameCallback;

struct CLuaError
{
	const char *data;
};

GarrysMod::Lua::ILuaInterface *lua = 0;

#ifdef LUAERROR_SERVER
#ifdef _WIN32
#define Push_Entity_signature "\x55\x8b\xec\x83\xec\x14\x83\x3d\x2A\x2A\x2A\x2A\x2A\x74\x2A\x8b\x4d\x08"
#endif
typedef void ( *Push_Entity_t ) ( CBaseEntity *entity );
Push_Entity_t Push_Entity;

#ifdef _WIN32
#define HandleClientLuaError_signature "\x55\x8b\xec\x83\xec\x08\x8b\x0d\x2A\x2A\x2A\x2A\x8b\x11\x53\x56"
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

#ifdef _WIN32
#define CLuaGameCallback__LuaError_signature "\x55\x8b\xec\x81\xec\x2A\x2A\x2A\x2A\x53\x56\x57\x33\xdb\x53\x89"
typedef void ( __thiscall *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );
#else
typedef void ( __cdecl *CLuaGameCallback__LuaError_t )( CLuaGameCallback *callback, CLuaError *error );
#endif
MologieDetours::Detour<CLuaGameCallback__LuaError_t> *CLuaGameCallback__LuaError_detour = 0;
CLuaGameCallback__LuaError_t CLuaGameCallback__LuaError;
#ifdef _WIN32
void __fastcall CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, void *fuckthis, CLuaError *error )
#else
void __cdecl CLuaGameCallback__LuaError_d( CLuaGameCallback *callback, CLuaError *error )
#endif
{
	const char *strerr = error->data;
	std::vector<lua_Debug> stack;
	lua_Debug dbg = { };
	int level = 1;
	while( lua->GetStack( level, &dbg ) == 1 )
	{
		if( lua->GetInfo( "Slnu", &dbg ) == 0 )
		{
			break;
		}

		level++;
		stack.push_back( dbg );
		memset( &dbg, 0, sizeof( dbg ) );
	}

	int stacksize = stack.size( );
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
						lua->Msg( "[LuaError hook error] %s\n", lua->GetString( ) );
						lua->Pop( 3 );
						return ( CLuaGameCallback__LuaError_detour->GetOriginalFunction( ) )( callback, error );
					}
				}
				else
				{
					if( lua->PCall( 3, 1, 0 ) != 0 )
					{
						lua->Msg( "[LuaError hook error] %s\n", lua->GetString( ) );
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



#ifdef _WIN32
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
	lua = (GarrysMod::Lua::ILuaInterface *)LUA;

	SymbolFinder symfinder;

#ifdef _WIN32
#ifdef LUAERROR_SERVER
	HMODULE server = LoadLibrary( "server.dll" );
	if( server != 0 )
	{
		HandleClientLuaError = (HandleClientLuaError_t)symfinder.FindPattern( server, HandleClientLuaError_signature, strlen( HandleClientLuaError_signature ) );
		if( HandleClientLuaError == 0 )
		{
			FreeLibrary( server );
			server = 0;
			LuaError( "Unable to detour function HandleClientLuaError." );
		}

		Push_Entity = (Push_Entity_t)symfinder.FindPattern( server, Push_Entity_signature, strlen( Push_Entity_signature ) );
		if( Push_Entity == 0 )
		{
			FreeLibrary( server );
			server = 0;
			LuaError( "Unable to find function Push_Entity." );
		}

		CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)symfinder.FindPattern( server, CLuaGameCallback__LuaError_signature, strlen( CLuaGameCallback__LuaError_signature ) );
		if( CLuaGameCallback__LuaError == 0 )
		{
			FreeLibrary( server );
			server = 0;
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (server.dll)." );
		}
	}
	else
	{
		LuaError( "Couldn't open server.dll." );
	}

	if( server != 0 )
	{
		FreeLibrary( server );
		server = 0;
	}
#elif defined LUAERROR_CLIENT
	HMODULE client = LoadLibrary( "client.dll" );
	if( client != 0 )
	{
		CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)symfinder.FindPattern( client, CLuaGameCallback__LuaError_signature, strlen( CLuaGameCallback__LuaError_signature ) );
		if( CLuaGameCallback__LuaError == 0 )
		{
			FreeLibrary( client );
			client = 0;
			LuaError( "Unable to detour function CLuaGameCallback::LuaError (client.dll)." );
		}
	}
	else
	{
		LuaError( "Couldn't open client.dll." );
	}

	if( client != 0 )
	{
		FreeLibrary( client );
		client = 0;
	}
#endif
#else
#define garrysmod_bin_path "garrysmod/bin/"
#ifdef __linux
#define server_file "server_srv.so"
#define client_file "client_srv.so"
#define FUNC_NAME_PREFIX ""
#elif defined __APPLE__
#define server_file "server.dylib"
#define client_file "client.dylib"
#define FUNC_NAME_PREFIX "_"
#endif
#define HandleClientLuaError_name FUNC_NAME_PREFIX "_Z20HandleClientLuaErrorP11CBasePlayerPKc"
#define Push_Entity_name FUNC_NAME_PREFIX "_Z11Push_EntityP11CBaseEntity"
#define CLuaGameCallback__LuaError_name FUNC_NAME_PREFIX "_ZN16CLuaGameCallback8LuaErrorEP9CLuaError"

#ifdef LUAERROR_SERVER
	void *server = dlopen( garrysmod_bin_path server_file, RTLD_NOW | RTLD_LOCAL );
	if( server != 0 )
	{
		HandleClientLuaError = (HandleClientLuaError_t)symfinder.FindSymbol( server, HandleClientLuaError_name );
		if( HandleClientLuaError == 0 )
		{
			dlclose( server );
			server = 0;
			LuaError( "Unable to detour function HandleClientLuaError." );
		}

		Push_Entity = (Push_Entity_t)symfinder.FindSymbol( server, Push_Entity_name );
		if( Push_Entity == 0 )
		{
			dlclose( server );
			server = 0;
			LuaError( "Unable to find function Push_Entity." );
		}

		CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)symfinder.FindSymbol( server, CLuaGameCallback__LuaError_name );
		if( CLuaGameCallback__LuaError == 0 )
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
#elif defined LUAERROR_CLIENT
	void *client = dlopen( garrysmod_bin_path client_file, RTLD_NOW | RTLD_LOCAL );
	if( client != 0 )
	{
		CLuaGameCallback__LuaError = (CLuaGameCallback__LuaError_t)symfinder.FindSymbol( client, CLuaGameCallback__LuaError_name );
		if( CLuaGameCallback__LuaError == 0 )
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

#ifdef LUAERROR_SERVER
	if( HandleClientLuaError != 0 )
	{
		HandleClientLuaError_detour = new MologieDetours::Detour<HandleClientLuaError_t>( HandleClientLuaError, HandleClientLuaError_d );
	}
#endif

	if( CLuaGameCallback__LuaError != 0 )
	{
		CLuaGameCallback__LuaError_detour = new MologieDetours::Detour<CLuaGameCallback__LuaError_t>( CLuaGameCallback__LuaError, (CLuaGameCallback__LuaError_t)CLuaGameCallback__LuaError_d );
	}

	lua->Msg( "[LuaError] Successfully loaded. Created by Daniel.\n" );
	return 0;
}

GMOD_MODULE_CLOSE( )
{
#ifdef LUAERROR_SERVER
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

	lua->Msg( "[LuaError] Successfully unloaded. Thank you for using LuaError. Created by Daniel.\n" );
	return 0;
}
