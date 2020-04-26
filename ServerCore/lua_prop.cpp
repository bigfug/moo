#include "lua_prop.h"

#include "property.h"
#include "lua_moo.h"
#include "lua_task.h"
#include "lua_object.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "objectmanager.h"
#include "object.h"
#include "connectionmanager.h"
#include "connection.h"
#include "inputsinkset.h"
#include "inputsinkedittext.h"

#include "changeset/propertysetowner.h"
#include "changeset/propertysetread.h"
#include "changeset/propertysetwrite.h"
#include "changeset/propertysetchange.h"
#include "changeset/connectionnotify.h"

const char	*lua_prop::mLuaName = "moo.prop";

LuaMap		 lua_prop::mLuaMap;

const luaL_Reg lua_prop::mLuaStatic[] =
{
	{ 0, 0 }
};

const luaL_Reg lua_prop::mLuaInstance[] =
{
	{ "__index", lua_prop::luaGet },
	{ "__newindex", lua_prop::luaSet },
	{ "__gc", lua_prop::luaGC },
	{ 0, 0 }
};

const luaL_Reg lua_prop::mLuaInstanceFunctions[] =
{
	{ "dump", lua_prop::luaDump },
	{ "program", lua_prop::luaProgram },
	{ "edit", lua_prop::luaEdit },
	{ "value", lua_prop::luaValue },
	{ 0, 0 }
};

void lua_prop::initialise()
{
	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}
}

void lua_prop::luaRegisterState( lua_State *L )
{
	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaInstance, 0 );

	lua_pop( L, 1 );
}

void lua_prop::lua_pushproperty( lua_State *L, Property *pProperty )
{
	luaProp		*P = reinterpret_cast<luaProp *>( lua_newuserdata( L, sizeof( luaProp ) ) );

	if( P == 0 )
	{
		luaL_error( L, "out of memory" );

		return;
	}

	P->mObjectId = OBJECT_NONE;
	P->mName     = new QString();
	P->mProperty = pProperty;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

void lua_prop::lua_pushproperty( lua_State *L, ObjectId pObjectId, const QString &pName, Property *pProperty )
{
	luaProp		*P = reinterpret_cast<luaProp *>( lua_newuserdata( L, sizeof( luaProp ) ) );

	if( P == 0 )
	{
		luaL_error( L, "out of memory" );

		return;
	}

	P->mObjectId = pObjectId;
	P->mName     = new QString( pName );
	P->mProperty = pProperty;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

lua_prop::luaProp *lua_prop::arg( lua_State *L, int pIndex )
{
	luaProp		*P = static_cast<luaProp *>( luaL_checkudata( L, pIndex, mLuaName ) );

	if( !P )
	{
		throw( mooException( E_ARGS, "property expected" ) );
	}

	return( P );
}

int lua_prop::luaGC( lua_State *L )
{
	luaProp		*P = arg( L );

	if( P->mObjectId == OBJECT_NONE )
	{
		delete( P->mProperty );
	}

	P->mObjectId = OBJECT_NONE;
	P->mProperty = 0;

	if( P->mName != 0 )
	{
		delete( P->mName );
	}

	return( 0 );
}

int lua_prop::luaGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	luaL_checkstring( L, 2 );

	try
	{
		luaProp				*LP = arg( L );
		Property			*P = LP->mProperty;
		const char			*s = luaL_checkstring( L, 2 );

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		if( !Command->isOwner( P ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "no access" );
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
			lua_pushstring( L, LP->mName->toLatin1() );

			return( 1 );
		}

		if( strcmp( s, "owner" ) == 0 )
		{
			lua_object::lua_pushobjectid( L, P->owner() );

			return( 1 );
		}

		if( strcmp( s, "r" ) == 0 )
		{
			lua_pushboolean( L, P->read() );

			return( 1 );
		}

		if( strcmp( s, "w" ) == 0 )
		{
			lua_pushboolean( L, P->write() );

			return( 1 );
		}

		if( strcmp( s, "c" ) == 0 )
		{
			lua_pushboolean( L, P->change() );

			return( 1 );
		}

		// Look for function in mLuaMap

//		lua_CFunction	 F;

//		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
//		{
//			lua_pushcfunction( L, F );

//			return( 1 );
//		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_prop::luaSet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );
	luaL_checkany( L, 3 );

	try
	{
		luaProp				*LP = arg( L );

		if( LP == 0 )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		if( lua_type( L, 2 ) != LUA_TSTRING )
		{
			throw( mooException( E_ARGS, "property name is not a string" ) );
		}

		const Task			&T = Command->task();
		Object				*Player = ObjectManager::instance()->object( T.player() );

		Property			*P = LP->mProperty;
		Object				*O = ObjectManager::o( LP->mObjectId );
		const char			*N = lua_tostring( L, 2 );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		if( strcmp( N, "name" ) == 0 )
		{
			throw( mooException( E_PERM, "can't set property name" ) );
		}

		if( strcmp( N, "owner" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			Object				*D = lua_object::argObj( L, 3 );

			Command->changeAdd( new change::PropertySetOwner( P, D->id() ) );

			return( 0 );
		}

		if( strcmp( N, "r" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			Command->changeAdd( new change::PropertySetRead( P, v ) );

			return( 0 );
		}

		if( strcmp( N, "w" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			Command->changeAdd( new change::PropertySetWrite( P, v ) );

			return( 0 );
		}

		if( strcmp( N, "c" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			Command->changeAdd( new change::PropertySetChange( P, v ) );

			return( 0 );
		}

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( N ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

void lua_prop::luaNewRecurse( lua_State *L, int pIdx, QVariant &pVariant )
{
	switch( lua_type( L, pIdx ) )
	{
		case LUA_TNUMBER:
			pVariant.setValue( double( lua_tonumber( L, pIdx ) ) );
			break;

		case LUA_TSTRING:
			pVariant.setValue( QString( lua_tostring( L, pIdx ) ) );
			break;

		case LUA_TBOOLEAN:
			pVariant.setValue( bool( lua_toboolean( L, pIdx ) ) );
			break;

		case LUA_TUSERDATA:
			{
				lua_object::luaHandle	*H = (lua_object::luaHandle *)luaL_testudata( L, pIdx, lua_object::luaHandle::mLuaName );

				if( H )
				{
					QVariant		V;

					V.setValue( *H );

					pVariant.setValue( V );
				}
			}
			break;

		case LUA_TTABLE:
			{
				QVariantMap		VarMap;

				lua_pushnil( L );		// first key

				while( lua_next( L, pIdx ) != 0 )
				{
					QString			K;

					switch( lua_type( L, -2 ) )
					{
						case LUA_TNUMBER:
							K = QString::number( lua_tointeger( L, -2 ) );
							break;

						case LUA_TSTRING:
							K = lua_tostring( L, -2 );
							break;

						default:
							lua_pop( L, 1 );
							continue;
					}

					QVariant		V;

					luaNewRecurse( L, pIdx + 2, V );

					VarMap.insert( K, V );

					/* removes 'value'; keeps 'key' for next iteration */

					lua_pop( L, 1 );
				}

				pVariant = VarMap;
			}
			break;

		default:
			luaL_error( L, "unknown type for property" );
			break;
	}
}

int lua_prop::luaDump( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		luaProp				*LP = arg( L );

		if( LP == 0 )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		const Task			&T = Command->task();
		Object				*Player = ObjectManager::instance()->object( T.player() );

		Property			*P = LP->mProperty;
		Object				*O = ObjectManager::o( LP->mObjectId );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( P == 0 )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		if( !isWizard && !isOwner && !P->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read property" ) );
		}

		if( P->value().type() != QVariant::String )
		{
			throw( mooException( E_TYPE, "can only dump string types" ) );
		}

		QStringList		Program = P->value().toString().split( "\n" );

		for( QStringList::iterator it = Program.begin() ; it != Program.end() ; it++ )
		{
			Command->changeAdd( new change::ConnectionNotify( C, *it ) );
		}

		Command->changeAdd( new change::ConnectionNotify( C, "." ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_prop::luaProgram( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaProp				*LP = arg( L );

		if( LP == 0 )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		const Task			&T = Command->task();
		Object				*Player = ObjectManager::instance()->object( T.player() );

		Property			*P = LP->mProperty;
		Object				*O = ObjectManager::o( LP->mObjectId );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( P == 0 )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		if( !isWizard && !isOwner && !P->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read property" ) );
		}

		if( P->value().type() != QVariant::String )
		{
			throw( mooException( E_TYPE, "can only dump string types" ) );
		}

		InputSinkSet	*IS = new InputSinkSet( C, O->id(), *LP->mName );

		if( IS == 0 )
		{
			return( 0 );
		}

		C->pushInputSink( IS );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_prop::luaEdit(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		luaProp				*LP = arg( L );

		if( LP == 0 )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		const Task			&T = Command->task();
		Object				*Player = ObjectManager::instance()->object( T.player() );

		Property			*P = LP->mProperty;
		Object				*O = ObjectManager::o( LP->mObjectId );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( P == 0 )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		if( !isWizard && !isOwner && !P->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read property" ) );
		}

		if( P->value().type() != QVariant::String )
		{
			throw( mooException( E_TYPE, "can only edit string types" ) );
		}

		QStringList		Text = P->value().toString().split( "\n" );

		InputSinkEditText	*IS = new InputSinkEditText( C, O->id(), P->name(), Text );

		if( !IS )
		{
			return( 0 );
		}

		C->pushInputSink( IS );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_prop::luaValue( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		luaProp				*LP = arg( L );

		if( LP == 0 )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		Property			*P = LP->mProperty;

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		if( !Command->isWizard() && !Command->isOwner( P ) && !P->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read property" ) );
		}

		luaL_pushvariant( L, P->value() );

		return( 1 );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}
