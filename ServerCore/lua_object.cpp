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
#include "lua_text.h"
#include "mooexception.h"

#include "changeset/objectaliasadd.h"
#include "changeset/objectaliasdelete.h"
#include "changeset/objectsetname.h"
#include "changeset/objectsetowner.h"
#include "changeset/objectsetread.h"
#include "changeset/objectsetwrite.h"
#include "changeset/objectsetfertile.h"
#include "changeset/objectsetplayer.h"
#include "changeset/objectsetprogrammer.h"
#include "changeset/objectsetwizard.h"
#include "changeset/objectsetproperty.h"
#include "changeset/objectverbadd.h"
#include "changeset/objectverbdelete.h"
#include "changeset/objectpropadd.h"
#include "changeset/objectpropclear.h"
#include "changeset/objectpropdelete.h"
#include "changeset/objectsetmodule.h"
#include "changeset/connectionnotify.h"

//-----------------------------------------------------------------------------
// Lua state
//-----------------------------------------------------------------------------

const char	*lua_object::luaHandle::mLuaName    = "moo.object";
const char	*lua_object::luaHandle::mTypeName   = "lua_object::luaHandle";

LuaMap		lua_object::mLuaMap;

const luaL_Reg lua_object::mLuaStatic[] =
{
	{ "create", lua_object::luaCreate },
	{ 0, 0 }
};

const luaL_Reg lua_object::mLuaInstance[] =
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
	{ "emit", lua_object::luaEmit },
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
	{ "isChildOf", lua_object::luaIsChildOf },
	{ "isParentOf", lua_object::luaIsParentOf },
	{ "is_child_of", lua_object::luaIsChildOf },
	{ "is_parent_of", lua_object::luaIsParentOf },
	{ "is_valid", lua_object::luaIsValid },
	{ "has_verb", lua_object::luaHasVerb },
	{ "has_prop", lua_object::luaHasProp },
	{ 0, 0 }
};

const QMap<QString,lua_object::Fields>		lua_object::mFieldMap =
{
	{ "id", ID },
	{ "__tostring", TO_STRING },
	{ "aliases", ALIASES },
	{ "name", NAME },
	{ "owner", OWNER },
	{ "parent", PARENT },
	{ "module", MODULE },
	{ "location", LOCATION },
	{ "connection", CONNECTION },
	{ "contents", CONTENTS },
	{ "player", PLAYER },
	{ "programmer", PROGRAMMER },
	{ "wizard", WIZARD },
	{ "r", READ },
	{ "w", WRITE },
	{ "f", FERTILE },
	{ "read", READ },
	{ "write", WRITE },
	{ "fertile", FERTILE }
};

void lua_object::luaRegisterState( lua_State *L )
{
	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, luaHandle::mLuaName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaInstance, 0 );

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
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		ObjectManager		&OM = *ObjectManager::instance();
		ObjectId			 ParentId = OBJECT_UNSPECIFIED;
		ObjectId			 OwnerId = OBJECT_UNSPECIFIED;
		const int			 argc = lua_gettop( L );
		Object				*objObject;

		if( argc >= 1 )
		{
			if( lua_isnumber( L, 1 ) )
			{
				if( ( ParentId = lua_tointeger( L, 1 ) ) != OBJECT_NONE )
				{
					if( !OM.object( ParentId ) )
					{
						throw( mooException( E_VARNF, QString( "unknown parent object id: %1" ).arg( ParentId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 1 ) )
			{
				ParentId = argId( L, 1 );

				if( ParentId != OBJECT_NONE && !OM.object( ParentId ) )
				{
					throw( mooException( E_VARNF, QString( "unknown parent object" ) ) );
				}
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
				if( ( OwnerId = lua_tointeger( L, 2 ) ) != OBJECT_NONE )
				{
					if( !OM.object( OwnerId ) )
					{
						throw( mooException( E_VARNF, QString( "unknown owner object id: %1" ).arg( ParentId ) ) );
					}
				}
			}
			else if( lua_isuserdata( L, 2 ) )
			{
				OwnerId = argId( L, 2 );

				if( OwnerId != OBJECT_NONE && !OM.object( OwnerId ) )
				{
					throw( mooException( E_VARNF, QString( "unknown owner object" ) ) );
				}
			}
			else
			{
				throw( mooException( E_TYPE, "ObjectId or Object expected for owner" ) );
			}
		}

		//qDebug() << "create: ParentId:" << ParentId << "OwnerId:" << OwnerId;

		ObjectId id = ObjectLogic::create( *Command, Command->permissions(), ParentId, OwnerId );

		if( ( objObject = OM.object( id ) ) != 0 )
		{
			// The owner of the new object is either the programmer (if owner is not provided)
			// the new object itself (if owner was given as #-1)
			// or owner (otherwise).

			if( argc < 2 )
			{
				objObject->setOwner( Command->task().permissions() );
			}
			else if( OwnerId == OBJECT_NONE )
			{
				objObject->setOwner( objObject->id() );
			}
			else
			{
				objObject->setOwner( OwnerId );
			}

			Verb		*FndVrb;
			Object		*FndObj;

			if( objObject->verbFind( "initialise", &FndVrb, &FndObj ) )
			{
				Command->verbCall( FndVrb, 0, objObject->id() );
			}

			Connection	*CON = ConnectionManager::instance()->connection( Command->connectionId() );

			if( CON )
			{
				CON->lastCreatedObjectId();

				CON->setLastCreatedObjectId( id );
			}
		}

		lua_pushobjectid( L, id );

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

int lua_object::luaGC( lua_State *L )
{
	Q_UNUSED( L )

	// Object      *O = argObj( L );

	//delete H->O;

	return( 0 );
}

int lua_object::luaGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		const char			*s = luaL_checkstring( L, 2 );
		lua_object::Fields	 Field = mFieldMap.value( QString::fromLatin1( s ) );

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		// Only support

		Object              *O = argObj( L );

		if( !O )
		{
			if( Field == ID )
			{
				lua_pushinteger( L, OBJECT_NONE );

				return( 1 );
			}

			if( Field == TO_STRING )
			{
				lua_pushcfunction( L, lua_object::luaToString );

				return( 1 );
			}

			throw( mooException( E_INVARG, "invalid object" ) );
		}

		switch( Field )
		{
			case ID:
				{
					lua_pushinteger( L, O->id() );

					return( 1 );
				}
				break;

			case TO_STRING:
				{
					lua_pushcfunction( L, lua_object::luaToString );

					return( 1 );
				}
				break;

			case ALIASES:
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
				break;

			case NAME:
				{
					lua_pushstring( L, O->name().toLatin1() );

					return( 1 );
				}
				break;

			case OWNER:
				{
					lua_pushobjectid( L, O->owner() );

					return( 1 );
				}
				break;

			case PARENT:
				{
					lua_pushobjectid( L, O->parent() );

					return( 1 );
				}
				break;

			case MODULE:
				{
					lua_pushobjectid( L, O->module() );

					return( 1 );
				}
				break;

			case LOCATION:
				{
					lua_pushobjectid( L, O->location() );

					return( 1 );
				}
				break;

			case CONNECTION:
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
				break;

			case CONTENTS:
				{
					lua_newtable( L );

					for( int i = 0 ; i < O->contents().size() ; i++ )
					{
						ObjectId	OID = O->contents().at( i );

						if( OID != OBJECT_NONE )
						{
							lua_pushinteger( L, i + 1 );
							lua_pushobjectid( L, O->contents().at( i ) );
							lua_settable( L, -3 );
						}
					}

					return( 1 );
				}
				break;

			case PLAYER:
				{
					lua_pushboolean( L, O->player() );

					return( 1 );
				}
				break;

			case PROGRAMMER:
				{
					lua_pushboolean( L, O->programmer() );

					return( 1 );
				}
				break;

			case WIZARD:
				{
					lua_pushboolean( L, O->wizard() );

					return( 1 );
				}
				break;

			case READ:
				{
					lua_pushboolean( L, O->read() );

					return( 1 );
				}
				break;

			case WRITE:
				{
					lua_pushboolean( L, O->write() );

					return( 1 );
				}
				break;

			case FERTILE:
				{
					lua_pushboolean( L, O->fertile() );

					return( 1 );
				}
				break;

			case UNKNOWN:
				break;
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

			if( !FndVrb->execute() )
			{
				throw mooException( E_PERM, "verb doesn't have execute bit set" );
			}

			lua_pushstring( L, s );
			lua_verb::lua_pushverb( L, FndVrb );
			lua_pushcclosure( L, lua_object::luaVerbCall, 2 );

			return( 1 );
		}

		// Look for a property on this object

		Property                *P = O->prop( s );

		if( P || ( P = O->propParent( QString( s ) ) ) != 0 )
		{
			if( P->type() == QVariant::Map )
			{
				return( lua_prop::lua_pushpropindex( L, O->id(), P ) );
			}
			else
			{
				return( luaL_pushvariant( L, P->value() ) );
			}
		}

		// Nothing found
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

int lua_object::luaSet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		Command->taskDump( "lua_object::luaSet", Command->task() );

		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		Object				*O = argObj( L );

		if( !O || !O->valid() )
		{
			throw( mooException( E_PERM, QString( "object (#%1) is invalid" ).arg( argId( L ) ) ) );
		}

		const char			*N = luaL_checkstring( L, 2 );

		switch( mFieldMap.value( QString::fromLatin1( N ) ) )
		{
			case ID:
				{
					throw( mooException( E_PERM, "can't set object id" ) );
				}
				break;

			case TO_STRING:
				{
					throw( mooException( E_PERM, "can't set __tostring" ) );
				}
				break;

			case ALIASES:
				{
					throw( mooException( E_PERM, "can't set aliases" ) );
				}
				break;

			case NAME:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					const char			*V = luaL_checkstring( L, 3 );

					if( O->name().compare( V ) )
					{
						Command->changeAdd( new change::ObjectSetName( O, V ) );
					}

					return( 0 );
				}
				break;

			case OWNER:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					ObjectId		OID = argId( L, 3 );

					if( O->owner() != OID )
					{
						Command->changeAdd( new change::ObjectSetOwner( O, argId( L, 3 ) ) );
					}

					return( 0 );
				}
				break;

			case PARENT:
				{
					ObjectId		OID = argId( L, 3 );

					if( O->parent() != OID )
					{
						ObjectLogic::chparent( *Command, O->id(), OID );
					}

					return( 0 );
				}
				break;

			case LOCATION:
				{
					ObjectId		OID = argId( L, 3 );

					if( O->location() != OID )
					{
						ObjectLogic::move( *Command, Command->permissions(), O->id(), OID );
					}

					return( 0 );
				}
				break;

			case CONNECTION:
				{
					throw( mooException( E_PERM, "can't set object connection" ) );
				}
				break;

			case MODULE:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					ObjectId		OID = argId( L, 3 );

					if( O->module() != OID )
					{
						Command->changeAdd( new change::ObjectSetModule( O, OID ) );
					}

					return( 0 );
				}
				break;

			case CONTENTS:
				{
					throw( mooException( E_PERM, "can't set object contents" ) );
				}
				break;

			case PLAYER:
				{
					if( !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->player() != V )
					{
						Command->changeAdd( new change::ObjectSetPlayer( O, V ) );
					}

					return( 0 );
				}
				break;

			case PROGRAMMER:
				{
					if( !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->programmer() != V )
					{
						Command->changeAdd( new change::ObjectSetProgrammer( O, V ) );
					}

					return( 0 );
				}
				break;

			case WIZARD:
				{
					if( !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->wizard() != V )
					{
						Command->changeAdd( new change::ObjectSetWizard( O, V ) );
					}

					return( 0 );
				}
				break;

			case READ:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->read() != V )
					{
						Command->changeAdd( new change::ObjectSetRead( O, V ) );
					}

					return( 0 );
				}
				break;

			case WRITE:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->write() != V )
					{
						Command->changeAdd( new change::ObjectSetWrite( O, V ) );
					}

					return( 0 );
				}
				break;

			case FERTILE:
				{
					if( !Command->isOwner( O ) && !Command->isWizard() )
					{
						throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
					}

					bool		V = lua_toboolean( L, 3 );

					if( O->fertile() != V )
					{
						Command->changeAdd( new change::ObjectSetFertile( O, V ) );
					}

					return( 0 );
				}
				break;

			case UNKNOWN:
				break;
		}

		Property		*FndPrp;
		Object			*FndObj;

		if( !O->propFind( N, &FndPrp, &FndObj ) )
		{
			throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( N ) ) );
		}

		if( !Command->isWizard() )
		{
			if( FndObj->id() != O->id() )
			{
				// found prop on parent object

				if( !FndPrp->change() )
				{
					if( !FndPrp->write() && !Command->isOwner( FndPrp ) )
					{
						throw mooException( E_PERM, "no access to property" );
					}
				}
			}
			else
			{
				if( !FndPrp->write() && !Command->isOwner( FndPrp ) )
				{
					throw mooException( E_PERM, "no access to property" );
				}
			}
		}

		QVariant		V = lua_util::lua_tovariant( L, FndPrp, 3 );

		Command->changeAdd( new change::ObjectSetProperty( O, FndPrp->name(), V ) );
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

int lua_object::luaToString( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		QString				 N = QString( "#%1" ).arg( O ? O->id() : OBJECT_NONE );

		lua_pushstring( L, N.toLatin1() );

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

int lua_object::luaEQ(lua_State *L)
{
	ObjectId	ID1 = argId( L, 1 );
	ObjectId	ID2 = argId( L, 2 );

	lua_pushboolean( L, ID1 == ID2 );

	return( 1 );
}

int lua_object::luaObject( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	ObjectId	OID = luaL_checkinteger( L, 1 );

	try
	{
		lua_pushobjectid( L, OID );

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

int lua_object::luaVerb( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "invalid programmer" );
		}

		const QString		 VerbName = luaL_checkstring( L, 2 );

		Object				*O = argObj( L );

		if( !O->read() && !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, QString( "can't read verb '%1' (not owner or elevated)" ).arg( VerbName ) );
		}

		Object				*FO;
		Verb				*FV;

		if( O->verbFind( VerbName, &FV, &FO ) )
		{
			lua_verb::lua_pushverb( L, FV );

			return( 1 );
		}
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

int lua_object::luaVerbAdd( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		Object				*O = argObj( L );
		QString				 VerbName( lua_tostring( L, 2 ) );
		Verb				 V;

		if( !Command->isOwner( O ) && !Command->isWizard() )
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

		V.setOwner( Command->permissions() );

		Command->changeAdd( new change::ObjectVerbAdd( O, VerbName, V ) );
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

int lua_object::luaVerbDel( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		Object				*O = argObj( L );
		QString				 VerbName( lua_tostring( L, 2 ) );

		if( !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "programmer doesn't own object" );
		}

		if( VerbName.isEmpty() )
		{
			throw mooException( E_INVARG, "verb name is not defined" );
		}

		Verb				*V = O->verb( VerbName );

		if( !V )
		{
			throw mooException( E_INVARG, "verb not defined on object" );
		}

		Command->changeAdd( new change::ObjectVerbDelete( O, VerbName ) );
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

int lua_object::luaVerbCall( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		const Task				&PrvT = Command->task();
		Task					 CurT = PrvT;
		Object					*O = argObj( L );
		const char				*s = lua_tostring( L, lua_upvalueindex( 1 ) );
		QString					 n( s );
		lua_verb::luaVerb		*v = (lua_verb::luaVerb *)lua_touserdata( L, lua_upvalueindex( 2 ) );
		int						 Error = 0;
		int						 ArgCnt = lua_gettop( L ) - 1;

		const int			 c1 = lua_gettop( L ) - ArgCnt;

		if( ( Error = v->mVerb->lua_pushverb( L ) ) != 0 )
		{
			return( 1 );
		}

		if( ArgCnt )
		{
			lua_insert( L, -1 - ArgCnt );
		}

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
		CurT.setPermissions( v->mVerb->owner() );
		CurT.setVerbObject( v->mVerb->object() );

		Command->taskDump( "luaVerbCall()", CurT );

		Command->taskPush( CurT );

		if( ( Error = lua_pcall( L, ArgCnt, LUA_MULTRET, 0 ) ) != 0 )
		{
			QString		ErrMsg = QString( lua_tostring( L, -1 ) );

			Connection	*CON = ConnectionManager::instance()->connection( Command->connectionId() );

			if( CON )
			{
				CON->notify( ErrMsg );
			}

			lua_pop( L, 1 );
		}

		int				c2 = lua_gettop( L );

		int				ResCnt = c2 - c1;

		Command->taskPop();

		// and return any results

		return( ResCnt );
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
	lua_task			*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );
	luaL_checkany( L, 3 );

	try
	{
		ObjectManager		&OM = *ObjectManager::instance();
		Object				*O = argObj( L );
		Property			 P;
		QString				 PropName( lua_tostring( L, 2 ) );
		Object				*O2;
		Property			*P2;
		QList<ObjectId>		 ObjLst;

		if( !O->write() && !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_INVARG, "programmer does not have write permission on object" );
		}

		if( O->prop( PropName ) )
		{
			throw mooException( E_INVARG, "property already exists" );
		}

		O->ancestors( ObjLst );

		for( ObjectId id : ObjLst )
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

		for( ObjectId id : ObjLst )
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

		P.setOwner( Command->permissions() );

		P.setValue( V );

		//lua_pushproperty( L, OBJECT_NONE, QString( lua_tostring( L, 1 ) ), P );

		Command->changeAdd( new change::ObjectPropAdd( O, PropName, P ) );

		return( 0 );
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

// Removes the property named prop-name from the given object and
// all of its descendants.

int lua_object::luaPropDel( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		QString				 PropName = QString( lua_tostring( L, 2 ) );
		Property			*P;

		// If object is not valid, then E_INVARG is raised.

		if( !O || !O->valid() )
		{
			throw mooException( E_INVARG, "object is invalid" );
		}

		// If the programmer does not have write permission on object
		// then E_PERM is raised.

		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is not valid" );
		}

		if( !Command->isWizard() && !Command->isOwner( O ) )
		{
			throw mooException( E_PERM, "programmer doesn't have access" );
		}

		// If object does not directly define a property named prop-name
		// (as opposed to inheriting one from its parent),
		// then E_PROPNF is raised.

		P = O->prop( PropName );

		if( !P || P->parent() != OBJECT_NONE )
		{
			throw mooException( E_PROPNF, "prop not defined on this object" );
		}

		Command->changeAdd( new change::ObjectPropDelete( O, PropName ) );

		return( 0 );
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

int lua_object::luaPropClear(lua_State *L)
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		QString				 PropName = QString( lua_tostring( L, 2 ) );
		Property			*P;

		// If object is not valid, then E_INVARG is raised.

		if( !O || !O->valid() )
		{
			throw mooException( E_INVARG, "object is invalid" );
		}

		// If the programmer does not have write permission on object
		// then E_PERM is raised.

		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is not valid" );
		}

		if( !Command->isWizard() && !Command->isOwner( O ) )
		{
			throw mooException( E_PERM, "programmer doesn't have access" );
		}

		// If object does not directly define a property named prop-name
		// (as opposed to inheriting one from its parent),
		// then E_PROPNF is raised.

		P = O->prop( PropName );

		if( !P )
		{
			throw mooException( E_PROPNF, "prop not defined on this object" );
		}

		if( P->parent() == OBJECT_NONE )
		{
			throw mooException( E_PROPNF, "prop is defined on this object" );
		}

		Command->changeAdd( new change::ObjectPropClear( O, PropName ) );

		return( 0 );
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

int lua_object::luaNotify( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	luaL_checkany( L, 2 );

	try
	{
		Object				*O = argObj( L );
		ConnectionManager	&CM  = *ConnectionManager::instance();
		ConnectionId		 CID = CM.fromPlayer( O->id() );
		Connection			*CON = ConnectionManager::instance()->connection( CID );

		if( CON )
		{
			Command->changeAdd( new change::ConnectionNotify( CON, lua_text::processString( L, O, 2 ) ) );
		}
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

int lua_object::luaProps( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_object::luaVerbs( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_object::luaFind( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_object::luaIsChildOf( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object						*O = argObj( L, 1 );
		const ObjectId				 P = argId( L, 2 );

		if( !O || O->id() == P )
		{
			lua_pushboolean( L, false );

			return( 1 );
		}

		while( O && O->id() != P )
		{
			O = ObjectManager::o( O->parent() );
		}

		lua_pushboolean( L, bool( O ) );

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

int lua_object::luaIsParentOf( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		const ObjectId				 P = argId( L, 1 );
		Object						*O = argObj( L, 2 );

		if( !O || O->id() == P )
		{
			lua_pushboolean( L, false );

			return( 1 );
		}

		while( O && O->id() != P )
		{
			O = ObjectManager::o( O->parent() );
		}

		lua_pushboolean( L, bool( O ) );

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

int lua_object::luaIsValid( lua_State *L )
{
	luaL_checkany( L, 1 );

	lua_object::luaHandle	*H = (lua_object::luaHandle *)luaL_testudata( L, 1, lua_object::luaHandle::mLuaName );

	lua_pushboolean( L, H && H->O >= 0 );

	return( 1 );
}

int lua_object::luaHasVerb( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		Object				*O = argObj( L );
		QString				 N = QString( lua_tostring( L, 2 ) );
		Object				*FO;
		Verb				*FV;

		lua_pushboolean( L, O->verbFind( N, &FV, &FO ) );

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

int lua_object::luaHasProp( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		Object				*O = argObj( L );
		QString				 N = QString( lua_tostring( L, 2 ) );
		Object				*FO;
		Property			*FP;

		lua_pushboolean( L, O->propFind( N, &FP, &FO ) );

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

int lua_object::luaChildren( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_object::luaPlayers( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_object::luaChild( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

// The given object is destroyed, irrevocably

int lua_object::luaRecycle( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );

		ObjectLogic::recycle( *Command, Command->permissions(), O->id() );
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

//-----------------------------------------------------------------------------
// Lua support functions
//-----------------------------------------------------------------------------

ObjectId lua_object::argId( lua_State *L, int pIndex )
{
	luaHandle *H = (luaHandle *)luaL_testudata( L, pIndex, lua_object::luaHandle::mLuaName );

	if( !H )
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

	if( !H )
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
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "invalid programmer" );
		}

		const char			*N = luaL_checkstring( L, 2 );

		Object				*O = argObj( L );

		if( !O->read() && !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, QString( "can't read property '%1' (not owner or elevated)" ).arg( QString::fromLatin1( N ) ) );
		}

		Object				*FO;
		Property			*FP;

		if( O->propFind( N, &FP, &FO ) )
		{
			lua_prop::lua_pushproperty( L, FO->id(), QString( N ), FP );

			return( 1 );
		}
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

int lua_object::luaAliasAdd(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		const char			*N = luaL_checkstring( L, -1 );

		if( !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		Command->changeAdd( new change::ObjectAliasAdd( O, N ) );
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

int lua_object::luaAliasDel(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		const char			*N = luaL_checkstring( L, -1 );

		if( !Command->isOwner( O ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		Command->changeAdd( new change::ObjectAliasDelete( O, N ) );
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

//----------------------------------------------------------------------------
// Signal/Slot support

int lua_object::luaEmit( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O = argObj( L );
		const char			*N = luaL_checkstring( L, 2 );

		QVector<QPair<ObjectId,QString>>		SignalMap = ObjectManager::instance()->objectSignals( O->id(), QString::fromLatin1( N ) );

		for( const QPair<ObjectId,QString> &c : SignalMap )
		{
			Object			*FndObj = Q_NULLPTR;
			Verb			*FndVrb = Q_NULLPTR;

			Object			*DstObj = ObjectManager::o( c.first );

			if( DstObj->verbFind( c.second, &FndVrb, &FndObj ) )
			{
				int r = Command->verbCall( FndVrb, lua_gettop( L ) - 2, DstObj->id(), Command->task().object() );

				lua_pop( L, r );
			}
		}
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
