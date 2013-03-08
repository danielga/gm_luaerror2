#ifndef ILUAINTERFACE_H
#define ILUAINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

#include "GarrysMod/Lua/LuaBase.h"

class ILuaInterface : public GarrysMod::Lua::ILuaBase
{
public:
	virtual void Nope1( ) = 0;
	virtual void Nope2( ) = 0;
	virtual void Nope3( ) = 0;
	virtual void Nope4( ) = 0;
	virtual lua_State *GetLuaState( ) = 0;
	virtual void Nope5( ) = 0;
	virtual void Nope6( ) = 0;
	virtual void Nope7( ) = 0;
	virtual void Nope8( ) = 0;
	virtual void Nope9( ) = 0;
	virtual void Nope10( ) = 0;
	virtual void Nope11( ) = 0;
	virtual void Nope12( ) = 0;
	virtual void Nope13( ) = 0;
	virtual void Nope14( ) = 0;
	virtual void Nope15( ) = 0;
	virtual void Nope16( ) = 0;
	virtual void Nope17( ) = 0;
	virtual void Nope18( ) = 0;
	virtual void Nope19( ) = 0;
	virtual void Nope20( ) = 0;
	virtual void Nope21( ) = 0;
	virtual void Nope22( ) = 0;
	virtual void Nope23( ) = 0;
	virtual void Nope24( ) = 0;
	virtual bool IsServer( ) = 0;
	virtual bool IsClient( ) = 0;
	virtual bool IsDedicatedServer( ) = 0;
};

#endif