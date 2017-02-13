#include "lua_object.h"
#include "objectmanager.h"
#include "task.h"
#include "connection.h"
#include "connectionmanager.h"
#include "lua_moo.h"
#include "lua_task.h"
#include "lua_verb.h"
#include "lua_prop.h"
#include "lua_connection.h"
#include "task.h"
#include "objectlogic.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <QDebug>
#include "lua_utilities.h"
#include "mooexception.h"

//-----------------------------------------------------------------------------
// Lua state
//-----------------------------------------------------------------------------

const char	*lua_object::luaHandle::mLuaName    = "moo.object";

LuaMap		lua_object::mLuaMap;

const luaL_Reg lua_object::mLuaStatic[] =
{
	{ "create", lua_object::luaCreate },
	{ 0, 0 }
};

const luaL_reg lua_object::mLuaInstance[] =
{
	{ "__index", lua_object::luaGet },
	{ "__newindex", lua_object::luaSet },
	{ "__gc", lua_object::luaGC },
	{ "__eq", lua_object::luaEQ },
	{ "__tostring", lua_object::luaToString },
	{ 0, 0 }
};

const luaL_Reg lua_object::mLuaInstanceFunctions[] =
{
	{ "aliasadd", lua_object::luaAliasAdd },
	{ "aliasdel", lua_object::luaAliasDel },
	{ "child", lua_object::luaChild },
	{ "children", lua_object::luaChildren },
	{ "verb", lua_object::luaVerb },
	{ "prop", lua_object::luaProperty },
	{ "recycle", lua_object::luaRecycle },
	{ "propadd", lua_object::luaPropAdd },
	{ "propdel", lua_object::luaPropDel },
	{ "propclr", lua_object::luaPropClear },
	{ "verbadd", lua_object::luaVerbAdd },
	{ "verbdel", lua_object::luaVerbDel },
	{ "notify", lua_object::luaNotify },
	{ "players", lua_object::luaPlayers },
	{ "props", lua_object::luaProps },
	{ "verbs", lua_object::luaVerbs },
	{ "find", lua_object::luaFind },
	{ 0, 0 }
};

void lua_object::luaRegisterState( lua_State *L )
{
	// Create our global 'o()' function for fast object access

	lua_pushcfunction( L, lua_object::luaObject );
	lua_setglobal( L, "o" );

	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, lua_object::luaHandle::mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	lua_pushstring( L, "__tostring" );
	lua_pushcfunction( L, lua_object::luaToString );
	lua_settable( L, -3 );  /* metatable.__tostring = luaToString */

	luaL_openlib( L, NULL, lua_object::mLuaInstance, 0 );

	lua_pop( L, 1 );
}


QDataStream &operator<<(QDataStream &out, const lua_object::luaHandle &myObj)
{
	out << myObj.O;

	return( out );
}

QDataStream &operator>>(QDataStream &in, lua_object::luaHandle &myObj)
{
	in >> myObj.O;

	return( in );
}


void lua_object::initialise( void )
{
	qRegisterMetaType<lua_object::luaHandle>( "luaHandle" );

	qRegisterMetaTypeStreamOperators<lua_object::luaHandle>( "luaHandle" );

	lua_moo::addFunctions( mLuaStatic );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}
}

//-----------------------------------------------------------------------------
// Lua functions
//-----------------------------------------------------------------------------

// Creates and returns a new object whose parent is parent and whose owner

int lua_object::luaCreate( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		ObjectManager		&OM = *ObjectManager::instance();
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		ObjectId			 ParentId = OBJECT_UNSPECIFIED;
		ObjectId			 OwnerId = OBJECT_UNSPECIFIED;
		Object				*objOwner = 0;
		Object				*objParent = 0;
		const int			 argc = lua_gettop( L );
		Object				*objObject;

		if( argc >= 1 )
		{
			if( lua_isnumber( L, 1 ) )
			{
				if( ( ParentId = lua_tointeger( L, 1 ) ) != -1 )
				{
					if( ( objParent = OM.object( ParentId ) ) == 0 )
					{
						throw( mooException( E_VARNF, QString( "unknown parent object id: %1" ).arg( ParentId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 1 ) )
			{
				if( ( objParent = argObj( L, 1 ) ) == 0 )
				{
					throw( mooException( E_VARNF, QString( "unknown parent object" ) ) );
				}

				ParentId  = objParent->id();
			}
			else
			{
				throw( mooException( E_TYPE, "ObjectId or Object expected for parent" ) );
			}
		}

		if( argc >= 2 )
		{
			if( lua_isnumber( L, 2 ) )
			{
				if( ( OwnerId = lua_tointeger( L, 2 ) ) != -1 )
				{
					if( ( objOwner = OM.object( OwnerId ) ) == 0 )
					{
						throw( mooException( E_VARNF, QString( "unknown owner object id: %1" ).arg( ParentId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 2 ) )
			{
				if( ( objOwner = argObj( L, 2 ) ) == 0 )
				{
					throw( mooException( E_VARNF, QString( "unknown owner object" ) ) );
				}

				OwnerId  = objOwner->id();
			}
			else
			{
				throw( mooException( E_TYPE, "ObjectId or Object expected for owner" ) );
			}
		}

		//qDebug() << "create: ParentId:" << ParentId << "OwnerId:" << OwnerId;

		ObjectId id = ObjectLogic::create( *Command, T.programmer(), ParentId, OwnerId );

		if( ( objObject = OM.object( id ) ) != 0 )
		{
			// The owner of the new object is either the programmer (if owner is not provided)
			// the new object itself (if owner was given as #-1)
			// or owner (otherwise).

//			if( argc < 2 )
//			{
//				objObject->setOwner( T.player() );
//			}
//			else if( OwnerId == -1 )
//			{
//				objObject->setOwner( objObject->id() );
//			}
//			else
//			{
//				objObject->setOwner( OwnerId );
//			}

			Verb		*FndVrb;
			Object		*FndObj;

			if( objObject->verbFind( "initialise", &FndVrb, &FndObj ) )
			{
				Task		T = Command->task();

				T.setObject( objObject->id() );

				Command->verbCall( T, FndVrb, 0 );
			}

			Connection	*CON = ConnectionManager::instance()->connection( Command->connectionid() );

			if( CON )
			{
				CON->setLastCreatedObjectId( id );
			}
		}

		lua_pushobjectid( L, id );

		return( 1 );
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

int lua_object::luaGC( lua_State *L )
{
	Q_UNUSED( L )

	// Object      *O = argObj( L );

	//delete H->O;

	return( 0 );
}

int lua_object::luaGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object              *O = argObj( L );
		const char			*s = luaL_checkstring( L, 2 );

		if( O == 0 )
		{
			if( strcmp( s, "id" ) == 0 )
			{
				lua_pushinteger( L, OBJECT_NONE );

				return( 1 );
			}

			throw( mooException( E_TYPE, "invalid object" ) );
		}

		Object				*PRG = ObjectManager::o( T.programmer() );

		if( PRG == 0 )
		{
			throw( mooException( E_TYPE, "invalid programmer" ) );
		}

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		if( strcmp( s, "__tostring" ) == 0 )
		{
			lua_pushcfunction( L, lua_object::luaToString );

			return( 1 );
		}

		// Look for a property on this object

		if( strcmp( s, "id" ) == 0 )
		{
			lua_pushinteger( L, O->id() );

			return( 1 );
		}

		if( strcmp( s, "aliases" ) == 0 )
		{
			const QStringList    AliasList = O->aliases();

			lua_newtable( L );

			for( int i = 0 ; i < AliasList.size() ; i++ )
			{
				lua_pushstring( L, AliasList[ i ].toLatin1() );

				lua_rawseti( L, -2, i + 1 );
			}

			return( 1 );
		}

		if( strcmp( s, "name" ) == 0 )
		{
			lua_pushstring( L, O->name().toLatin1() );

			return( 1 );
		}

		if( strcmp( s, "owner" ) == 0 )
		{
			lua_pushobjectid( L, O->owner() );

			return( 1 );
		}

		if( strcmp( s, "parent" ) == 0 )
		{
			lua_pushobjectid( L, O->parent() );

			return( 1 );
		}

		if( strcmp( s, "location" ) == 0 )
		{
			lua_pushobjectid( L, O->location() );

			return( 1 );
		}

		if( strcmp( s, "connection" ) == 0 )
		{
			if( O->player() )
			{
				Connection			*C = ConnectionManager::instance()->connection( ConnectionManager::instance()->fromPlayer( O->id() ) );

				if( C )
				{
					lua_connection::lua_pushconnection( L, C );

					return( 1 );
				}
			}

			lua_pushnil( L );

			return( 1 );
		}

		if( strcmp( s, "contents" ) == 0 )
		{
			lua_newtable( L );

			for( int i = 0 ; i < O->contents().size() ; i++ )
			{
				lua_pushinteger( L, i + 1 );
				lua_pushobjectid( L, O->contents().at( i ) );
				lua_settable( L, -3 );
			}

			return( 1 );
		}

		if( strcmp( s, "player" ) == 0 )
		{
			lua_pushboolean( L, O->player() );

			return( 1 );
		}

		if( strcmp( s, "programmer" ) == 0 )
		{
			lua_pushboolean( L, O->programmer() );

			return( 1 );
		}

		if( strcmp( s, "wizard" ) == 0 )
		{
			lua_pushboolean( L, O->wizard() );

			return( 1 );
		}

		if( strcmp( s, "r" ) == 0 )
		{
			lua_pushboolean( L, O->read() );

			return( 1 );
		}

		if( strcmp( s, "w" ) == 0 )
		{
			lua_pushboolean( L, O->write() );

			return( 1 );
		}

		if( strcmp( s, "f" ) == 0 )
		{
			lua_pushboolean( L, O->fertile() );

			return( 1 );
		}

		// Look for a verb on this object

		Verb		*FndVrb;
		Object		*FndObj;

		//if( O->verbFind( s, &FndVrb, &FndObj, T.directObjectId(), T.preposition(), T.indirectObjectId() ) )
		//if( O->verbFind( s, &FndVrb, &FndObj, O->id(), QString(), O->id() ) )
		if( O->verbFind( s, &FndVrb, &FndObj ) )
		{
//			qDebug() << "lua_object::luaGet" << s;

//			lua_moo::stackReverseDump( L );

			lua_pushstring( L, s );
			lua_verb::lua_pushverb( L, FndVrb );
			lua_pushcclosure( L, lua_object::luaVerbCall, 2 );

			return( 1 );
		}

		// Look for a property on this object

		Property                *P = O->prop( s );

		if( P != 0 || ( P = O->propParent( QString( s ) ) ) != 0 )
		{
			return( luaL_pushvariant( L, P->value() ) );
		}

		// Nothing found
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

int lua_object::luaSet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*PRG = ObjectManager::o( Command->programmer() );

		if( PRG == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

//		Object				*Player = ObjectManager::instance()->object( T.player() );
		Object				*O = argObj( L );

		if( O == 0 || !O->valid() )
		{
			throw( mooException( E_PERM, QString( "object (#%1) is invalid" ).arg( argId( L ) ) ) );
		}

		const char			*N = luaL_checkstring( L, 2 );
		const bool			 isOwner  = ( PRG->id() == O->owner() );
		const bool			 isWizard = ( PRG->wizard() );

		if( strcmp( N, "id" ) == 0 )
		{
			throw( mooException( E_PERM, "can't set object id" ) );
		}

		if( strcmp( N, "name" ) == 0 )
		{
			const char			*V = luaL_checkstring( L, 3 );

			if( !isOwner && !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
			}

			O->setName( V );

			return( 0 );
		}

		if( strcmp( N, "owner" ) == 0 )
		{
			if( !isOwner && !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
			}

			Object				*V = argObj( L, 3 );

			O->setOwner( V->id() );

			return( 0 );
		}

		if( strcmp( N, "programmer" ) == 0 )
		{
			if( !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setProgrammer( V );

			return( 0 );
		}

		if( strcmp( N, "wizard" ) == 0 )
		{
			if( !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setWizard( V );

			return( 0 );
		}

		if( strcmp( N, "player" ) == 0 )
		{
			if( !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setPlayer( V );

			return( 0 );
		}

		if( strcmp( N, "r" ) == 0 )
		{
			if( !isOwner && !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setRead( V );

			return( 0 );
		}

		if( strcmp( N, "w" ) == 0 )
		{
			if( !isOwner && !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setWrite( V );

			return( 0 );
		}

		if( strcmp( N, "f" ) == 0 )
		{
			if( !isOwner && !isWizard )
			{
				throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
			}

			bool		V = lua_toboolean( L, 3 );

			O->setFertile( V );

			return( 0 );
		}

		if( strcmp( N, "location" ) == 0 )
		{
			Object				*ObjWhere   = 0;
			ObjectId			 ObjWhereId = -1;

			if( lua_isnumber( L, 3 ) )
			{
				if( ( ObjWhereId = lua_tointeger( L, 3 ) ) != -1 )
				{
					if( ( ObjWhere = ObjectManager::o( ObjWhereId ) ) == 0 )
					{
						throw( mooException( E_TYPE, QString( "unknown object %1" ).arg( ObjWhereId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 3 ) )
			{
				if( ( ObjWhere = argObj( L, 3 ) ) == 0 )
				{
					throw( mooException( E_VARNF, QString( "unknown location object" ) ) );
				}

				ObjWhereId = ObjWhere->id();
			}
			else
			{
				throw( mooException( E_TYPE, QString( "ObjectId or Object expected %1" ).arg( 2 ) ) );
			}

			ObjectLogic::move( *Command, T.programmer(), O->id(), ObjWhereId );

			return( 0 );
		}

		if( strcmp( N, "parent" ) == 0 )
		{
			ObjectId			 NewParentId = -1;
			Object				*NewParent   = 0;

			luaL_checkany( L, 3 );

			if( lua_isnumber( L, 3 ) )
			{
				if( ( NewParentId = lua_tointeger( L, 3 ) ) != -1 )
				{
					if( ( NewParent = ObjectManager::o( NewParentId ) ) == 0 )
					{
						throw( mooException( E_TYPE, QString( "unknown object %1" ).arg( NewParentId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 3 ) )
			{
				if( ( NewParent = argObj( L, 3 ) ) == 0 )
				{
					throw( mooException( E_VARNF, QString( "unknown parent object" ) ) );
				}

				NewParentId = NewParent->id();
			}
			else
			{
				throw( mooException( E_TYPE, QString( "ObjectId or Object expected %1" ).arg( 2 ) ) );
			}

			ObjectLogic::chparent( *Command, T.programmer(), O->id(), NewParentId );

			return( 0 );
		}

		Property		*FndPrp;
		Object			*FndObj;

		if( !O->propFind( N, &FndPrp, &FndObj ) )
		{
			throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( N ) ) );
		}

		if( !O->write() && FndPrp->owner() != PRG->id() && !isWizard )
		{
			throw( mooException( E_PERM, QString( "programmer (#%1) is not owner (#%2) or wizard of property (#%3)" ).arg( T.programmer() ).arg( O->owner() ).arg( FndPrp->owner() ) ) );
		}

		return( lua_prop::luaSetValue( L, N, O, FndPrp, FndObj, 3 ) );
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

int lua_object::luaToString( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		Object				*O = argObj( L );
		QString				 N = QString( "#%1" ).arg( O->id() );

		lua_pushstring( L, N.toLatin1() );

		return( 1 );
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

int lua_object::luaEQ(lua_State *L)
{
	ObjectId	ID1 = argId( L, 1 );
	ObjectId	ID2 = argId( L, 2 );

	lua_pushboolean( L, ID1 == ID2 );

	return( 1 );
}

int lua_object::luaObject( lua_State *L )
{
	bool		LuaErr = false;

	luaL_checkinteger( L, 1 );

	try
	{
		lua_pushobjectid( L, lua_tointeger( L, 1 ) );

		return( 1 );
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

int lua_object::luaVerb( lua_State *L )
{
	bool		LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object				*PRG = ObjectManager::o( T.programmer() );

		if( PRG == 0 )
		{
			throw mooException( E_PERM, "invalid programmer" );
		}

		Object				*O = argObj( L );

		if( !O->read() && PRG->id() != O->owner() && !PRG->wizard() )
		{
			throw mooException( E_PERM, "bad access" );
		}

		const QString		 VerbName = luaL_checkstring( L, 2 );
		Verb				*V = O->verb( VerbName );

		if( V == 0 )
		{
			throw mooException( E_PERM, "bad verb" );
		}

		lua_verb::lua_pushverb( L, V );

		return( 1 );
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

int lua_object::luaVerbAdd( lua_State *L )
{
	bool		LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*O = argObj( L );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 PlayerOwnsObject = ( Player == 0 ? false : Player->id() == O->owner() || Player->wizard() );
		QString				 VerbName( lua_tostring( L, 2 ) );
		Verb				 V;

		if( !PlayerOwnsObject )
		{
			throw mooException( E_PERM, "programmer doesn't own object" );
		}

		if( VerbName.isEmpty() )
		{
			throw mooException( E_INVARG, "verb name is not defined" );
		}

		if( O->verbMatch( VerbName ) )
		{
			throw mooException( E_INVARG, "verb name is already defined" );
		}

		V.initialise();

		V.setOwner( T.programmer() );
		V.setObject( O->id() );

		O->verbAdd( VerbName, V );

		lua_verb::lua_pushverb( L, O->verb( VerbName ) );

		return( 1 );
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

int lua_object::luaVerbDel(lua_State *L)
{
	bool		LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*O = argObj( L );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 PlayerOwnsObject = ( Player == 0 ? false : Player->id() == O->owner() || Player->wizard() );
		QString				 VerbName( lua_tostring( L, 2 ) );

		if( !PlayerOwnsObject )
		{
			throw mooException( E_PERM, "programmer doesn't own object" );
		}

		if( VerbName.isEmpty() )
		{
			throw mooException( E_INVARG, "verb name is not defined" );
		}

		Verb				*V = O->verb( VerbName );

		if( V == 0 )
		{
			throw mooException( E_INVARG, "verb not defined on object" );
		}

		O->verbDelete( VerbName );

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

int lua_object::luaVerbCall( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&PrvT = Command->task();
		Task				 CurT = PrvT;
		Object              *O = argObj( L );
		const char			*s = lua_tostring( L, lua_upvalueindex( 1 ) );
		QString				 n( s );
		lua_verb::luaVerb	*v = (lua_verb::luaVerb *)lua_touserdata( L, lua_upvalueindex( 2 ) );
		int					 Error = 0;
		int					 ArgCnt = lua_gettop( L ) - 1;

		const int			 c1 = lua_gettop( L ) - ArgCnt;

		//qDebug() << "lua_object::luaVerbCall" << n << ArgCnt << "args:";

		if( ( Error = v->mVerb->lua_pushverb( L ) ) != 0 )
		{
			return( 1 );
		}

		if( ArgCnt )
		{
			lua_insert( L, -1 - ArgCnt );
		}

		//lua_moo::stackReverseDump( L );

		/*
			this
				an object, the value of expr-0
			verb
				a string, the name used in calling this verb
			args
				a list, the values of expr-1, expr-2, etc.
			caller
				an object, the value of this in the calling verb
			player
				an object, the same value as it had initially in the calling verb or,
				if the calling verb is running with wizard permissions, the same as the
				current value in the calling verb.

			All other built-in variables (argstr, dobj, etc.) are initialized with the
			same values they have in the calling verb.
		*/

		CurT.setObject( O->id() );
		CurT.setVerb( n );
		CurT.setCaller( PrvT.object() );

		Object		*Wizard = ( CurT.programmer() == OBJECT_NONE ? 0 : ObjectManager::o( CurT.programmer() ) );

		if( CurT.programmer() != OBJECT_NONE && ( Wizard == 0 || !Wizard->wizard() ) )
		{
			CurT.setProgrammer( v->mVerb->owner() );
		}

		Command->taskPush( CurT );

		if( ( Error = lua_pcall( L, ArgCnt, LUA_MULTRET, 0 ) ) != 0 )
		{
			QString		ErrMsg = QString( lua_tostring( L, -1 ) );

			Connection	*CON = ConnectionManager::instance()->connection( Command->connectionid() );

			if( CON != 0 )
			{
				CON->notify( ErrMsg );
			}

			lua_pop( L, 1 );
		}

		int				c2 = lua_gettop( L );

		int				ResCnt = c2 - c1;

		//qDebug() << "lua_object::luaVerbCall" << n << ResCnt << "results:";

		//lua_moo::stackReverseDump( L );

		Command->taskPop();

		// and return any results

		return( ResCnt );
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


// Defines a new property on the given object, inherited by all
// of its descendants

// If object is not valid or info does not specify a valid owner
//   and well-formed permission bits
//   or object or its ancestors or descendants already defines
//   a property named prop-name, then E_INVARG is raised.
// If the programmer does not have write permission on object
//   or if the owner specified by info is not the programmer and
//   the programmer is not a wizard, then E_PERM is raised.

int lua_object::luaPropAdd( lua_State *L )
{
	bool				 LuaErr = false;

	luaL_checkstring( L, 2 );
	luaL_checkany( L, 3 );

	try
	{
		ObjectManager		&OM = *ObjectManager::instance();
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*O = argObj( L );
		Object				*PRG = ObjectManager::o( Command->programmer() );
		Property			 P;
		QString				 PropName( lua_tostring( L, 2 ) );
		Object				*O2;
		Property			*P2;
		QList<ObjectId>		 ObjLst;

		if( !O->write() && PRG != 0 && !PRG->wizard() && PRG->id() != O->owner() )
		{
			luaL_error( L, "programmer does not have write permission on object" );
		}

		if( O->prop( PropName ) != 0 )
		{
			throw mooException( E_INVARG, "property already exists" );
		}

		O->ancestors( ObjLst );

		foreach( ObjectId id, ObjLst )
		{
			if( ( O2 = OM.object( id ) ) == 0 )
			{
				continue;
			}

			if( ( P2 = O2->prop( PropName ) ) == 0 )
			{
				continue;
			}

			throw mooException( E_INVARG, "property already defined on ancestor" );
		}

		ObjLst.clear();

		O->descendants( ObjLst );

		foreach( ObjectId id, ObjLst )
		{
			if( ( O2 = OM.object( id ) ) == 0 )
			{
				continue;
			}

			if( ( P2 = O2->prop( PropName ) ) == 0 )
			{
				continue;
			}

			throw mooException( E_INVARG, "property already defined on descendent" );
		}

		QVariant	V;

		lua_prop::luaNewRecurse( L, 3, V );

		if( !V.isValid() )
		{
			throw mooException( E_INVARG, "value not valid" );
		}

		P.initialise();

		P.setOwner( T.programmer() );

		P.setValue( V );

		//lua_pushproperty( L, OBJECT_NONE, QString( lua_tostring( L, 1 ) ), P );

		O->propAdd( PropName, P );

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

	return( LuaErr ? lua_error( L ) : 0 );	}

// Removes the property named prop-name from the given object and
// all of its descendants.

int lua_object::luaPropDel( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		//ObjectManager		&OM	= *ObjectManager::instance();
		Object				*O = argObj( L );
		Object				*PRG = ObjectManager::o( T.programmer() );
		QString				 PropName = QString( lua_tostring( L, 2 ) );
		Property			*P;

		// If object is not valid, then E_INVARG is raised.

		if( O == 0 || !O->valid() )
		{
			throw mooException( E_INVARG, "object is invalid" );
		}

		// If the programmer does not have write permission on object
		// then E_PERM is raised.

		if( T.programmer() != OBJECT_NONE && ( PRG == 0 || !PRG->valid() ) )
		{
			throw mooException( E_PERM, "programmer is not valid" );
		}

		if( PRG != 0 && !PRG->wizard() && PRG->id() != O->owner() )
		{
			throw mooException( E_PERM, "programmer doesn't have access" );
		}

		// If object does not directly define a property named prop-name
		// (as opposed to inheriting one from its parent),
		// then E_PROPNF is raised.

		P = O->prop( PropName );

		if( P == 0 || P->parent() != OBJECT_NONE )
		{
			throw mooException( E_PROPNF, "prop not defined on this object" );
		}

		O->propDelete( PropName );

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

int lua_object::luaPropClear(lua_State *L)
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		//ObjectManager		&OM	= *ObjectManager::instance();
		Object				*O = argObj( L );
		Object				*PRG = ObjectManager::o( T.programmer() );
		QString				 PropName = QString( lua_tostring( L, 2 ) );
		Property			*P;

		// If object is not valid, then E_INVARG is raised.

		if( O == 0 || !O->valid() )
		{
			throw mooException( E_INVARG, "object is invalid" );
		}

		// If the programmer does not have write permission on object
		// then E_PERM is raised.

		if( T.programmer() != OBJECT_NONE && ( PRG == 0 || !PRG->valid() ) )
		{
			throw mooException( E_PERM, "programmer is not valid" );
		}

		if( PRG != 0 && !PRG->wizard() && PRG->id() != O->owner() )
		{
			throw mooException( E_PERM, "programmer doesn't have access" );
		}

		// If object does not directly define a property named prop-name
		// (as opposed to inheriting one from its parent),
		// then E_PROPNF is raised.

		P = O->prop( PropName );

		if( P == 0 )
		{
			throw mooException( E_PROPNF, "prop not defined on this object" );
		}

		if( P->parent() == OBJECT_NONE )
		{
			throw mooException( E_PROPNF, "prop is defined on this object" );
		}

		O->propClear( PropName );

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

int lua_object::luaNotify( lua_State *L )
{
	bool				 LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		Object				*O = argObj( L );
		ConnectionManager	&CM  = *ConnectionManager::instance();
		ConnectionId		 CID = CM.fromPlayer( O->id() );
		Connection			*CON = ConnectionManager::instance()->connection( CID );
		QString				 MSG = QString( lua_tostring( L, 2 ) );

		if( CON != 0 )
		{
			CON->notify( MSG );
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

int lua_object::luaProps( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		Object							*O = argObj( L );
		QMap<QString,Property>			&C = O->propmap();
		int								 i = 1;

		lua_newtable( L );

		for( QMap<QString,Property>::iterator it = C.begin() ; it != C.end() ; it++, i++ )
		{
			lua_pushinteger( L, i );
			lua_prop::lua_pushproperty( L, O->id(), it.key(), &it.value() );
			lua_settable( L, -3 );
		}

		return( 1 );
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

int lua_object::luaVerbs( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		Object						*O = argObj( L );
		QMap<QString,Verb>			&C = O->verbmap();
		int							 i = 1;

		lua_newtable( L );

		for( QMap<QString,Verb>::iterator it = C.begin() ; it != C.end() ; it++, i++ )
		{
			lua_pushinteger( L, i );
			lua_verb::lua_pushverb( L, &it.value() );
			lua_settable( L, -3 );
		}

		return( 1 );
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

int lua_object::luaFind( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		const QString				 N = luaL_checkstring( L, -1 );

		Object						*O = argObj( L );
		const QList<ObjectId>		&C = O->contents();
		int							 i = 1;
		ObjectList					 ObjLst;

		for( ObjectId id : C )
		{
			Object					*F = ObjectManager::o( id );

			if( !F )
			{
				continue;
			}

			if( F->name().compare( N, Qt::CaseInsensitive ) == 0 )
			{
				ObjLst << F;

				continue;
			}

			for( const QString &S : F->aliases() )
			{
				if( S.startsWith( N, Qt::CaseInsensitive ) )
				{
					ObjLst << F;

					break;
				}
			}
		}

		if( ObjLst.size() == 0 )
		{
			return( 0 );
		}

		if( ObjLst.size() == 1 )
		{
			lua_pushobject( L, ObjLst.first() );

			return( 1 );
		}

		lua_newtable( L );

		for( ObjectList::iterator it = ObjLst.begin() ; it != ObjLst.end() ; it++, i++ )
		{
			lua_pushinteger( L, i );
			lua_pushobject( L, *it );
			lua_settable( L, -3 );
		}

		return( 1 );
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

int lua_object::luaChildren( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		Object						*O = argObj( L );
		const QList<ObjectId>		&C = O->children();
		int							 i = 1;

		lua_newtable( L );

		for( QList<ObjectId>::const_iterator it = C.begin() ; it != C.end() ; it++, i++ )
		{
			lua_pushinteger( L, i );
			lua_pushobjectid( L, *it );
			lua_settable( L, -3 );
		}

		return( 1 );
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

int lua_object::luaPlayers( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		Object						*O = argObj( L );
		const QList<ObjectId>		&C = O->contents();
		int							 i = 1;

		lua_newtable( L );

		for( QList<ObjectId>::const_iterator it = C.begin() ; it != C.end() ; it++ )
		{
			Object	*CurObj = ObjectManager::o( *it );

			if( CurObj && CurObj->player() )
			{
				lua_pushinteger( L, i++ );
				lua_pushobjectid( L, *it );
				lua_settable( L, -3 );
			}
		}

		return( 1 );
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


int lua_object::luaChild( lua_State *L )
{
	bool				 LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		Object						*O = argObj( L );
		const QList<ObjectId>		&C = O->children();
		const QString				 S = QString( lua_tostring( L, 2 ) );

		for( QList<ObjectId>::const_iterator it = C.begin() ; it != C.end() ; it++ )
		{
			Object		*C = ObjectManager::o( *it );

			if( C == 0 || C->name().compare( S, Qt::CaseInsensitive ) != 0 )
			{
				continue;
			}

			lua_pushobjectid( L, *it );

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

// The given object is destroyed, irrevocably

int lua_object::luaRecycle( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*O = argObj( L );

		ObjectLogic::recycle( *Command, T.programmer(), O->id() );
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
// Lua support functions
//-----------------------------------------------------------------------------

ObjectId lua_object::argId( lua_State *L, int pIndex )
{
	luaHandle *H = (luaHandle *)luaL_testudata( L, pIndex, lua_object::luaHandle::mLuaName );

	if( H == 0 )
	{
		throw( mooException( E_VARNF, "unknown object id" ) );
	}

	return( H->O );
}

Object * lua_object::argObj( lua_State *L, int pIndex )
{
	return( ObjectManager::o( argId( L, pIndex ) ) );
}

int lua_object::lua_pushobject( lua_State *L, Object *O )
{
	lua_pushobjectid( L, O->id() );

	return( 1 );
}

int lua_object::lua_pushobjectid( lua_State *L, ObjectId I )
{
	luaHandle			*H = (luaHandle *)lua_newuserdata( L, sizeof( luaHandle ) );

	if( H == 0 )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	H->O = I;

	luaL_getmetatable( L, lua_object::luaHandle::mLuaName );
	lua_setmetatable( L, -2 );

	return( 1 );
}

int lua_object::luaProperty( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		//lua_task			*Command = lua_task::luaGetTask( L );
		//const Task			&T = Command->task();
		Object				*O = argObj( L );
		const char			*N = luaL_checkstring( L, 2 );
		Object				*FO;
		Property			*FP;

		if( O->propFind( N, &FP, &FO ) )
		{
			lua_prop::lua_pushproperty( L, FO->id(), QString( N ), FP );

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

int lua_object::luaAliasAdd(lua_State *L)
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object				*O = argObj( L );
		Object				*Player = ObjectManager::o( T.programmer() );

		if( Player == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		const char			*N = luaL_checkstring( L, -1 );

		if( Player->id() != O->owner() && !Player->wizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		O->aliasAdd( N );
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

int lua_object::luaAliasDel(lua_State *L)
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object				*O = argObj( L );
		Object				*Player = ObjectManager::o( T.programmer() );

		if( Player == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		const char			*N = luaL_checkstring( L, -1 );

		if( Player->id() != O->owner() && !Player->wizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		O->aliasDelete( N );
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
