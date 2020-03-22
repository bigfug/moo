#ifndef LUA_TEXT_H
#define LUA_TEXT_H

#include <lua.hpp>

#include "lua_utilities.h"

class Connection;

class lua_text
{
private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaPronounSubstitution( lua_State *L );

	static int luaBold( lua_State *L );

	static const luaL_Reg		 mLuaStatic[];

	friend class lua_moo;
};

#endif // LUA_TEXT_H
