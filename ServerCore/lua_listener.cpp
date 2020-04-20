#include "lua_listener.h"
#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"

const char	*lua_listener::mLuaName = "moo.listener";

const luaL_Reg lua_listener::mLuaStatic[] =
{
	{ "listeners", lua_listener::luaListeners },
	{ 0, 0 }
};

const luaL_Reg lua_listener::mLuaInstance[] =
{
	{ 0, 0 }
};

void lua_listener::initialise()
{
	lua_moo::addFunctions( mLuaStatic );
}

void lua_listener::luaRegisterState( lua_State *L )
{
	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaInstance, 0 );

	luaL_newlib( L, mLuaStatic );

	lua_pop( L, 1 );
}

void lua_listener::lua_pushlistener( lua_State *L, ListenerServer *O )
{
	luaListener			*H = (luaListener *)lua_newuserdata( L, sizeof( luaListener ) );

	if( !H )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	H->mListener = O;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

int lua_listener::luaListeners( lua_State *L )
{
	Q_UNUSED( L )

	return( 0 );
}

//-----------------------------------------------------------------------------

lua_listener::luaListener * lua_listener::arg( lua_State *L, int pIndex )
{
	luaListener *H = (luaListener *)luaL_testudata( L, pIndex, mLuaName );

	if( H == 0 )
	{
		throw( mooException( E_TYPE, QString( "'listener' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}
