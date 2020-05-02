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
#include "inputsink/inputsinkset.h"
#include "inputsink/inputsinkedittext.h"

#include "changeset/propertysetowner.h"
#include "changeset/propertysetread.h"
#include "changeset/propertysetwrite.h"
#include "changeset/propertysetchange.h"
#include "changeset/connectionnotify.h"
#include "changeset/objectsetproperty.h"

const char	*lua_prop::mLuaName = "moo.prop";
const char	*lua_prop::mLuaPropIndexName = "moo.propindex";

LuaMap		 lua_prop::mLuaMap;
LuaMap		 lua_prop::mLuaPropIndexMap;

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

const QMap<QString,lua_prop::Fields> lua_prop::mFieldMap =
{
	{ "name", NAME },
	{ "owner", OWNER },
	{ "r", READ },
	{ "w", WRITE },
	{ "c", CHANGE },
	{ "read", READ },
	{ "write", WRITE },
	{ "change", CHANGE },
};

const luaL_Reg lua_prop::mLuaPropIndexInstance[] =
{
	{ "__index", lua_prop::luaPropIndexGet },
	{ "__gc", lua_prop::luaPropIndexGC },
	{ "__len", lua_prop::luaPropIndexLen },
	{ 0, 0 }
};

const luaL_Reg lua_prop::mLuaPropIndexInstanceFunctions[] =
{
	{ "set", lua_prop::luaPropIndexSet },
	{ "clear", lua_prop::luaPropIndexClear },
	{ "value", lua_prop::luaPropIndexValue },
};

void lua_prop::initialise()
{
	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}

	for( const luaL_Reg &FP : mLuaPropIndexInstanceFunctions )
	{
		mLuaPropIndexMap[ FP.name ] = FP.func;
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

	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, mLuaPropIndexName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaPropIndexInstance, 0 );

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

		if( !Command->isWizardOrOwner( P ) )
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

		switch( mFieldMap.value( QString( s ) ) )
		{
			case NAME:
				{
					lua_pushstring( L, LP->mName->toLatin1() );

					return( 1 );
				}

			case OWNER:
				{
					lua_object::lua_pushobjectid( L, P->owner() );

					return( 1 );
				}

			case READ:
				{
					lua_pushboolean( L, P->read() );

					return( 1 );
				}

			case WRITE:
				{
					lua_pushboolean( L, P->write() );

					return( 1 );
				}

			case CHANGE:
				{
					lua_pushboolean( L, P->change() );

					return( 1 );
				}

			case UNKNOWN:
				break;
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

		if( !LP )
		{
			throw( mooException( E_PERM, "property is invalid" ) );
		}

		if( lua_type( L, 2 ) != LUA_TSTRING )
		{
			throw( mooException( E_ARGS, "property name is not a string" ) );
		}

		Property			*P = LP->mProperty;
		const char			*N = lua_tostring( L, 2 );

		switch( mFieldMap.value( QString( N ) ) )
		{
			case NAME:
				{
					throw( mooException( E_PERM, "can't set property name" ) );
				}

			case OWNER:
				{
					if( !Command->isWizardOrOwner( P ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					Object				*D = lua_object::argObj( L, 3 );

					Command->changeAdd( new change::PropertySetOwner( P, D->id() ) );

					return( 0 );
				}

			case READ:
				{
					if( !Command->isWizardOrOwner( P ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::PropertySetRead( P, v ) );

					return( 0 );
				}

			case WRITE:
				{
					if( !Command->isWizardOrOwner( P ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::PropertySetWrite( P, v ) );

					return( 0 );
				}

			case CHANGE:
				{
					if( !Command->isWizardOrOwner( P ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::PropertySetChange( P, v ) );

					return( 0 );
				}

			case UNKNOWN:
				break;
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

//---------------------------------------------------------------------------
// Property Index


int lua_prop::lua_pushpropindex( lua_State *L, ObjectId pObjectId, Property *pProperty )
{
	luaPropIndex	*P = reinterpret_cast<luaPropIndex *>( lua_newuserdata( L, sizeof( luaPropIndex ) ) );

	if( !P )
	{
		luaL_error( L, "out of memory" );

		return( 0 );
	}

	P->mProperty = pProperty;
	P->mObjectId = pObjectId;
	P->mIndex    = Q_NULLPTR;

	luaL_getmetatable( L, mLuaPropIndexName );
	lua_setmetatable( L, -2 );

	return( 1 );
}

int lua_prop::lua_pushpropindex( lua_State *L, ObjectId pObjectId, Property *pProperty, const lua_prop::luaPropIndexPath &pPath )
{
	luaPropIndex	*P = reinterpret_cast<luaPropIndex *>( lua_newuserdata( L, sizeof( luaPropIndex ) ) );

	if( !P )
	{
		luaL_error( L, "out of memory" );

		return( 0 );
	}

	P->mProperty = pProperty;
	P->mObjectId = pObjectId;
	P->mIndex    = new luaPropIndexPath( pPath );

	luaL_getmetatable( L, mLuaPropIndexName );
	lua_setmetatable( L, -2 );

	return( 1 );
}

lua_prop::luaPropIndex *lua_prop::propindex( lua_State *L, int pIndex )
{
	luaPropIndex		*P = static_cast<luaPropIndex *>( luaL_checkudata( L, pIndex, mLuaPropIndexName ) );

	if( !P )
	{
		throw( mooException( E_ARGS, "property index expected" ) );
	}

	return( P );
}


int lua_prop::luaPropIndexGC( lua_State *L )
{
	luaPropIndex		*P = propindex( L );

	P->mProperty = Q_NULLPTR;

	if( P->mIndex )
	{
		delete( P->mIndex );

		P->mIndex = Q_NULLPTR;
	}

	return( 0 );
}

int lua_prop::luaPropIndexGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		luaPropIndex		*LP = propindex( L );
		Property			*P = LP->mProperty;
		QString				 S = QString::fromLatin1( lua_tostring( L, 2 ) );

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaPropIndexMap.value( S, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		luaPropIndexPath	 Path;

		if( LP->mIndex )
		{
			Path = *LP->mIndex;
		}

		Path.append( S );

		QVariant			 V = P->value();

		for( QString e : Path )
		{
			if( V.type() == QVariant::Map )
			{
				QVariantMap		M = V.toMap();
				QVariantMap::const_iterator it = M.find( e );

				if( it == M.end() )
				{
					return( 0 );
				}

				V = *it;
			}
			else
			{
				throw( mooException( E_PROPNF, "map index not a map" ) );
			}
		}

		if( V.type() == QVariant::Map || V.type() == QVariant::List )
		{
			return( lua_pushpropindex( L, LP->mObjectId, P, Path ) );
		}

		return( luaL_pushvariant( L, V ) );
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

int lua_prop::luaPropIndexSet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );
	luaL_checkany( L, 3 );

	try
	{
		luaPropIndex		*LP = propindex( L );
		Property			*P = LP->mProperty;
		QString				 S = QString::fromLatin1( lua_tostring( L, 2 ) );

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		luaPropIndexPath	 Path;

		if( LP->mIndex )
		{
			Path = *LP->mIndex;
		}

		QVector<QPair<QVariantMap,QString>>	IndexList;

		Path.append( S );

		QVariant			 V = P->value();

		for( QString e : Path )
		{
			if( V.type() == QVariant::Map )
			{
				QVariantMap		M = V.toMap();

				IndexList << QPair<QVariantMap,QString>( M, e );

				V = M.value( e );
			}
			else
			{
				throw( mooException( E_PROPNF, "map index not a map" ) );
			}
		}

		QVariant			NewV;

		luaNewRecurse( L, 3, NewV );

		while( !IndexList.isEmpty() )
		{
			QPair<QVariantMap,QString>	Index = IndexList.takeLast();

			Index.first.insert( Index.second, NewV );

			NewV = Index.first;
		}

		Command->changeAdd( new change::ObjectSetProperty( ObjectManager::o( LP->mObjectId ), P->name(), NewV ) );
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

int lua_prop::luaPropIndexClear(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		luaPropIndex		*LP = propindex( L );
		Property			*P = LP->mProperty;
		QString				 S = QString::fromLatin1( lua_tostring( L, 2 ) );

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		luaPropIndexPath	 Path;

		if( LP->mIndex )
		{
			Path = *LP->mIndex;
		}

		QVector<QPair<QVariantMap,QString>>	IndexList;

		Path.append( S );

		QVariant			 V = P->value();

		for( QString e : Path )
		{
			if( V.type() == QVariant::Map )
			{
				QVariantMap		M = V.toMap();
				QVariantMap::const_iterator it = M.find( e );

				IndexList << QPair<QVariantMap,QString>( M, e );

				V = *it;
			}
		}

		QVariant			NewV;

		while( !IndexList.isEmpty() )
		{
			QPair<QVariantMap,QString>	Index = IndexList.takeLast();

			if( !NewV.isValid() )
			{
				Index.first.remove( Index.second );
			}
			else
			{
				Index.first.insert( Index.second, NewV );
			}

			NewV = Index.first;
		}

		Command->changeAdd( new change::ObjectSetProperty( ObjectManager::o( LP->mObjectId ), P->name(), NewV ) );
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

int lua_prop::luaPropIndexValue(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaPropIndex		*LP = propindex( L );
		Property			*P = LP->mProperty;

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		luaPropIndexPath	 Path;

		if( LP->mIndex )
		{
			Path = *LP->mIndex;
		}

		QVariant			 V = P->value();

		for( QString e : Path )
		{
			if( V.type() == QVariant::Map )
			{
				QVariantMap		M = V.toMap();
				QVariantMap::const_iterator it = M.find( e );

				if( it == M.end() )
				{
					return( 0 );
				}

				V = *it;
			}
			else
			{
				throw( mooException( E_PROPNF, "map index not a map" ) );
			}
		}

		return( luaL_pushvariant( L, V ) );
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

int lua_prop::luaPropIndexLen(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaPropIndex		*LP = propindex( L );
		Property			*P = LP->mProperty;

		if( !P )
		{
			throw( mooException( E_TYPE, "invalid property" ) );
		}

		luaPropIndexPath	 Path;

		if( LP->mIndex )
		{
			Path = *LP->mIndex;
		}

		QVariant			 V = P->value();

		for( QString e : Path )
		{
			if( V.type() == QVariant::Map )
			{
				QVariantMap		M = V.toMap();
				QVariantMap::const_iterator it = M.find( e );

				if( it == M.end() )
				{
					return( 0 );
				}

				V = *it;
			}
			else
			{
				throw( mooException( E_PROPNF, "map index not a map" ) );
			}
		}

		if( V.type() != QVariant::Map )
		{
			throw( mooException( E_PROPNF, "map index not a map" ) );
		}

		lua_pushinteger( L, V.toMap().size() );

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
