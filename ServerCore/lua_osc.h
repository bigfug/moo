#ifndef LUA_OSC_H
#define LUA_OSC_H

#include <lua.hpp>

#include "lua_utilities.h"

class lua_osc
{
private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static const luaL_Reg		 mLuaStatic[];

	friend class lua_moo;
};

#endif // LUA_OSC_H
