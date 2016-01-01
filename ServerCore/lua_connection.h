#ifndef LUA_CONNECTION_H
#define LUA_CONNECTION_H

#include <lua.hpp>

#include "lua_utilities.h"

class Connection;

class lua_connection
{
public:
	typedef struct luaConnection
	{
		Connection		*mConnection;
	} luaConnection;

	static void lua_pushconnection( lua_State *L, Connection *O );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaPlayer( lua_State *L );
	static int luaNotify( lua_State *L );
	static int luaBoot( lua_State *L );

	static int luaConnections( lua_State *L );
	static int luaCon( lua_State *L );

	static int luaGet( lua_State *L );

	static luaConnection *arg( lua_State *L, int pIndex = 1 );

	static const char			*mLuaName;

	static LuaMap				 mLuaMap;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
	static const luaL_Reg		 mLuaInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_CONNECTION_H
