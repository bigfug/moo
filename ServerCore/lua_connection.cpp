#include "lua_connection.h"
#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "connectionmanager.h"

const char	*lua_connection::mLuaName = "moo.connection";
LuaMap		lua_connection::mLuaMap;

const luaL_Reg lua_connection::mLuaStatic[] =
{
	{ "connections", lua_connection::luaConnections },
	{ "connection", lua_connection::luaCon },
	{ 0, 0 }
};

const luaL_Reg lua_connection::mLuaInstance[] =
{
	{ "__index", lua_connection::luaGet },
	{ "__newindex", lua_connection::luaSet },
	{ 0, 0 }
};

const luaL_Reg lua_connection::mLuaInstanceFunctions[] =
{
	{ "notify", lua_connection::luaNotify },
	{ "boot", lua_connection::luaBoot },
	{ "player", lua_connection::luaPlayer },
	{ 0, 0 }
};

void lua_connection::initialise()
{
	lua_moo::addGet( mLuaStatic );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}
}

void lua_connection::luaRegisterState( lua_State *L )
{
	// Create the moo.connection metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_connection::mLuaInstance, 0 );

	lua_pop( L, 1 );
}

void lua_connection::lua_pushconnection( lua_State *L, Connection *O )
{
	luaConnection			*H = (luaConnection *)lua_newuserdata( L, sizeof( luaConnection ) );

	if( H == 0 )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	H->mConnection = O;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

int lua_connection::luaPlayer( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaConnection	*Con = arg( L );

		if( Con != 0 )
		{
			lua_object::lua_pushobjectid( L, Con->mConnection->player() );

			return( 1 );
		}
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_connection::luaNotify( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		if( lua_gettop( L ) != 2 )
		{
			throw( mooException( E_ARGS, "wrong number of arguments" ) );
		}

		luaConnection	*Con = arg( L );
		const char		*Msg = lua_tolstring( L, 2, 0 );

		if( Msg )
		{
			throw( mooException( E_INVARG, "expected string" ) );
		}

		if( Con && Con->mConnection )
		{
			Con->mConnection->notify( QString( Msg ) );
		}

		return( 0 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_connection::luaBoot( lua_State *L )
{
	Q_UNUSED( L )

	return( 0 );
}

int lua_connection::luaConnections( lua_State *L )
{
	ConnectionManager			&CM = *ConnectionManager::instance();
	const ConnectionNodeMap		&NM = CM.connectionList();
	int							 i = 1;

	lua_newtable( L );

	for( ConnectionNodeMap::const_iterator it = NM.begin() ; it != NM.end() ; it++, i++ )
	{
		lua_pushinteger( L, i );
		lua_pushconnection( L, it.value() );
		lua_settable( L, -3 );
	}

	return( 1 );
}

int lua_connection::luaCon( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );
	Connection			*C = ConnectionManager::instance()->connection( Command->connectionid() );

	lua_pushconnection( L, C );

	return( 1 );
}

int lua_connection::luaGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		//		const Task			&T = lua_task::luaGetTask( L )->task();
		luaConnection		*LC = arg( L );
		Connection			*C = LC->mConnection;
		const char			*s = luaL_checkstring( L, 2 );
		//		Object				*O = ObjectManager::o( LP->mObjectId );
		//		Object				*Player = ObjectManager::o( T.player() );
		//		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		//		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		if( C == 0 )
		{
			throw( mooException( E_TYPE, "invalid connection" ) );
		}

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		if( strcmp( s, "name" ) == 0 )
		{
			lua_pushstring( L, C->name().toLatin1() );

			return( 1 );
		}

		if( strcmp( s, "player" ) == 0 )
		{
			lua_object::lua_pushobjectid( L, C->player() );

			return( 1 );
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_connection::luaSet(lua_State *L)
{
	bool		LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		luaConnection		*LC = arg( L );
		Connection			*C = LC->mConnection;
		const char			*s = luaL_checkstring( L, 2 );

		luaL_checkany( L, 3 );

		if( !C )
		{
			throw( mooException( E_TYPE, "invalid connection" ) );
		}

		if( strcmp( s, "player" ) == 0 )
		{
			Object				*PRG = ObjectManager::o( T.programmer() );

			if( !PRG || !PRG->wizard() )
			{
				throw mooException( E_PERM, "only wizards can do that" );
			}

			Object	*Player = lua_object::argObj( L, 3 );

			if( !Player || !Player->player() )
			{
				throw mooException( E_INVARG, "object is not valid, or not a player" );
			}

			ConnectionManager			&CM = *ConnectionManager::instance();

			CM.logon( C->id(), Player->id() );

			Player->setConnection( C->id() );

			return( 0 );
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

//-----------------------------------------------------------------------------

lua_connection::luaConnection * lua_connection::arg( lua_State *L, int pIndex )
{
	luaConnection *H = (luaConnection *)luaL_testudata( L, pIndex, mLuaName );

	if( H == 0 )
	{
		throw( mooException( E_TYPE, QString( "'connection' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

