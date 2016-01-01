#ifndef LUA_LISTENER_H
#define LUA_LISTENER_H

#include <lua.hpp>

class Listener;

class lua_listener
{
public:
	typedef struct luaListener
	{
		Listener		*mListener;
	} luaListener;

	static void lua_pushlistener( lua_State *L, Listener *O );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaListeners( lua_State *L );

	static luaListener *arg( lua_State *L, int pIndex = 1 );

	static const char			*mLuaName;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
};

#endif // LUA_LISTENER_H
