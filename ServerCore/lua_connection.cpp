#include "lua_connection.h"
#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "lua_text.h"
#include "connectionmanager.h"
#include "changeset/connectionnotify.h"
#include "changeset/connectionclose.h"

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
	{ 0, 0 }
};

const QMap<QString,lua_connection::Fields> lua_connection::mFieldMap =
{
	{ "id", ID },
	{ "NAME", NAME },
	{ "player", PLAYER },
	{ "object", OBJECT },
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
	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaInstance, 0 );

	lua_pop( L, 1 );
}

void lua_connection::lua_pushconnection( lua_State *L, Connection *O )
{
	luaConnection			*H = (luaConnection *)lua_newuserdata( L, sizeof( luaConnection ) );

	if( !H )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	H->mConnection = O;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

int lua_connection::luaNotify( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		if( lua_gettop( L ) < 2 )
		{
			throw( mooException( E_ARGS, "wrong number of arguments" ) );
		}

		luaConnection	*Con = arg( L );

		if( Con && Con->mConnection )
		{
			Command->changeAdd( new change::ConnectionNotify( Con->mConnection, lua_text::processString( L, ObjectManager::o( Con->mConnection->player() ), 2 ) ) );
		}
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( const std::exception &e )
	{
		Command->setException( mooException( E_EXCEPTION, e.what() ) );
	}

	return( Command->lua_pushexception() );
}

int lua_connection::luaBoot( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		if( !Command->isWizard() )
		{
			throw mooException( E_PERM, "only wizards can boot" );
		}

		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( C )
		{
			Command->changeAdd( new change::ConnectionClose( C ) );
		}
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( const std::exception &e )
	{
		Command->setException( mooException( E_EXCEPTION, e.what() ) );
	}

	return( Command->lua_pushexception() );
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
	Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

	lua_pushconnection( L, C );

	return( 1 );
}

int lua_connection::luaGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaConnection		*LC = arg( L );
		Connection			*C = LC->mConnection;
		const char			*s = luaL_checkstring( L, 2 );

		if( !C )
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

		switch( mFieldMap.value( QString( s ) ) )
		{
			case ID:
				{
					lua_pushinteger( L, C->id() );

					return( 1 );
				}
				break;

			case NAME:
				{
					lua_pushstring( L, C->name().toLatin1() );

					return( 1 );
				}
				break;

			case PLAYER:
				{
					lua_object::lua_pushobjectid( L, C->player() );

					return( 1 );
				}
				break;

			case OBJECT:
				{
					lua_object::lua_pushobjectid( L, C->object() );

					return( 1 );
				}
				break;
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "connection: property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( const std::exception &e )
	{
		Command->setException( mooException( E_EXCEPTION, e.what() ) );
	}

	return( Command->lua_pushexception() );
}

int lua_connection::luaSet(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaConnection		*LC = arg( L );
		Connection			*C = LC->mConnection;
		const char			*s = luaL_checkstring( L, 2 );

		luaL_checkany( L, 3 );

		if( !C )
		{
			throw( mooException( E_TYPE, "invalid connection" ) );
		}

		switch( mFieldMap.value( QString( s ) ) )
		{
			case ID:
				{
					throw( mooException( E_PERM, "Can't set id of connection" ) );
				}
				break;

			case NAME:
				{
					throw( mooException( E_PERM, "Can't set name of connection" ) );
				}
				break;

			case PLAYER:
				{
					Object	*Player = lua_object::argObj( L, 3 );

					if( !Player || !Player->player() )
					{
						throw mooException( E_INVARG, "connection: player object is not valid, or not a player" );
					}

					if( !Command->isWizard() )
					{
						throw mooException( E_PERM, "Only wizards can set the player on a connection" );
					}

					return( Command->login( Player ) );
				}
				break;

			case OBJECT:
				{
					throw( mooException( E_PERM, "Can't set object of connection" ) );
				}
				break;
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "connection: property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( const std::exception &e )
	{
		Command->setException( mooException( E_EXCEPTION, e.what() ) );
	}

	return( Command->lua_pushexception( lua_gettop( L ) ) );
}

//-----------------------------------------------------------------------------

lua_connection::luaConnection * lua_connection::arg( lua_State *L, int pIndex )
{
	luaConnection *H = (luaConnection *)luaL_testudata( L, pIndex, mLuaName );

	if( !H )
	{
		throw( mooException( E_TYPE, QString( "'connection' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

