#include "lua_json.h"

#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "connectionmanager.h"

const char	*lua_json::luaJsonArray::mLuaName    = "moo.json.array";
const char	*lua_json::luaJsonDocument::mLuaName = "moo.json.document";
const char	*lua_json::luaJsonObject::mLuaName   = "moo.json.object";

LuaMap		 lua_json::mLuaDocumentMap;
LuaMap		 lua_json::mLuaArrayMap;
LuaMap		 lua_json::mLuaObjectMap;

const luaL_Reg lua_json::mLuaStatic[] =
{
	{ "parse", lua_json::luaParse },
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaArrayInstance[] =
{
	{ "__index", lua_json::luaGetArray },
	{ "__ipairs", lua_json::luaArrayIPairs },
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaArrayInstanceFunctions[] =
{
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaObjectInstance[] =
{
	{ "__index", lua_json::luaObjectGet },
	{ "isObject", lua_json::luaObjectIsObject },
	{ "isArray", lua_json::luaObjectIsArray },
	{ "is_object", lua_json::luaObjectIsObject },
	{ "is_array", lua_json::luaObjectIsArray },
	{ "pairs", lua_json::luaObjectPairs },
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaObjectInstanceFunctions[] =
{
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaDocumentInstance[] =
{
	{ "__index", lua_json::luaDocumentGet },
	{ "isObject", lua_json::luaDocumentIsObject },
	{ "isArray", lua_json::luaDocumentIsArray },
	{ "object", lua_json::luaDocumentObject },
	{ "array", lua_json::luaDocumentArray },
	{ 0, 0 }
};

const luaL_Reg lua_json::mLuaDocumentInstanceFunctions[] =
{
	{ 0, 0 }
};

void lua_json::lua_pushjsondocument( lua_State *L, const QJsonDocument &pJsonDocument )
{
	luaJsonDocument			*UD = (luaJsonDocument *)lua_newuserdata( L, sizeof( luaJsonDocument ) );

	if( !UD )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	new( &UD->mJsonDocument ) QJsonDocument( pJsonDocument );

	luaL_getmetatable( L, luaJsonDocument::mLuaName );
	lua_setmetatable( L, -2 );
}

void lua_json::lua_pushjsonarray( lua_State *L, const QJsonArray &pJsonArray )
{
	luaJsonArray			*UD = (luaJsonArray *)lua_newuserdata( L, sizeof( luaJsonArray ) );

	if( !UD )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	new( &UD->mJsonArray ) QJsonArray( pJsonArray );
	new( &UD->mIterator )  QJsonArray::iterator();

	luaL_getmetatable( L, luaJsonArray::mLuaName );
	lua_setmetatable( L, -2 );
}

void lua_json::lua_pushjsonobject( lua_State *L, const QJsonObject &pJsonObject )
{
	luaJsonObject			*UD = (luaJsonObject *)lua_newuserdata( L, sizeof( luaJsonObject ) );

	if( !UD )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	new( &UD->mJsonObject ) QJsonObject( pJsonObject );
	new( &UD->mIterator )  QJsonObject::iterator();

	luaL_getmetatable( L, luaJsonObject::mLuaName );
	lua_setmetatable( L, -2 );
}

int lua_json::lua_pushjsonvalue( lua_State *L, const QJsonValue &V )
{
	if( V.isArray() )
	{
		lua_pushjsonarray( L, V.toArray() );

		return( 1 );
	}

	if( V.isBool() )
	{
		lua_pushboolean( L, V.toBool() );

		return( 1 );
	}

	if( V.isDouble() )
	{
		lua_pushnumber( L, V.toDouble() );

		return( 1 );
	}

	if( V.isObject() )
	{
		lua_pushjsonobject( L, V.toObject() );

		return( 1 );
	}

	if( V.isString() )
	{
		lua_pushstring( L, V.toString().toLatin1() );

		return( 1 );
	}

	return( 0 );
}

void lua_json::initialise()
{
	lua_moo::addFunctions( mLuaStatic );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaDocumentInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaDocumentMap[ FP->name ] = FP->func;
	}

	for( const luaL_Reg *FP = mLuaArrayInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaArrayMap[ FP->name ] = FP->func;
	}

	for( const luaL_Reg *FP = mLuaObjectInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaObjectMap[ FP->name ] = FP->func;
	}
}

void lua_json::luaRegisterState( lua_State *L )
{
	// Create the moo.connection metatables that is used for all objects

	luaL_newmetatable( L, luaJsonDocument::mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_json::mLuaDocumentInstance, 0 );

	lua_pop( L, 1 );

	// Create the moo.connection metatables that is used for all objects

	luaL_newmetatable( L, luaJsonArray::mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_json::mLuaArrayInstance, 0 );

	lua_pop( L, 1 );

	// Create the moo.connection metatables that is used for all objects

	luaL_newmetatable( L, luaJsonObject::mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_json::mLuaObjectInstance, 0 );

	lua_pop( L, 1 );
}

int lua_json::luaParse( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		QJsonDocument		 D;
		QJsonParseError		 E;
		size_t				 l;
		const char			*s = luaL_checklstring( L, 1, &l );

		D = QJsonDocument::fromJson( QByteArray::fromRawData( s, l ), &E );

		if( D.isNull() )
		{
			throw( mooException( E_ARGS, E.errorString() ) );

			return( 1 );
		}

		lua_pushjsondocument( L, D );

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

int lua_json::luaGetArray( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaJsonArray		*UD = array( L );
		int					 i  = luaL_checkinteger( L, 2 ) - 1;

		if( !UD )
		{
			throw( mooException( E_TYPE, "invalid JSON array" ) );
		}

		if( i < 0 || i >= UD->mJsonArray.size() )
		{
			throw( mooException( E_INVIND, "no entry" ) );
		}

		return( lua_pushjsonvalue( L, UD->mJsonArray.at( i ) ) );
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

int lua_json::luaObjectGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaJsonObject		*UD = object( L );
		const char			*s = luaL_checkstring( L, 2 );

		if( !UD )
		{
			throw( mooException( E_TYPE, "invalid JSON object" ) );
		}

		QString				 k = QString::fromLatin1( s );

		if( !UD->mJsonObject.contains( k ) )
		{
			throw( mooException( E_INVIND, "no entry" ) );
		}

		return( lua_pushjsonvalue( L, UD->mJsonObject.value( s ) ) );
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

int lua_json::luaDocumentIsObject(lua_State *L)
{
	luaJsonDocument		*UD = document( L );

	lua_pushboolean( L, UD->mJsonDocument.isObject() );

	return( 1 );
}

int lua_json::luaDocumentIsArray(lua_State *L)
{
	luaJsonDocument		*UD = document( L );

	lua_pushboolean( L, UD->mJsonDocument.isArray() );

	return( 1 );
}

int lua_json::luaDocumentObject(lua_State *L)
{
	luaJsonDocument		*UD = document( L );

	if( !UD->mJsonDocument.isObject() )
	{
		throw( mooException( E_INVARG, "document isn't object" ) );
	}

	lua_pushjsonobject( L, UD->mJsonDocument.object() );

	return( 1 );
}

int lua_json::luaDocumentArray(lua_State *L)
{
	luaJsonDocument		*UD = document( L );

	if( !UD->mJsonDocument.isArray() )
	{
		throw( mooException( E_INVARG, "document isn't array" ) );
	}

	lua_pushjsonarray( L, UD->mJsonDocument.array() );

	return( 1 );
}

int lua_json::luaArrayIPairs( lua_State *L )
{
	luaJsonArray		*UD = array( L, 1 );
	int					 i  = lua_gettop( L ) > 1 ? luaL_checkinteger( L, 2 ) : 0;

	i = i + 1;

	if( i >= UD->mJsonArray.count() )
	{
		return( 0 );
	}

	QJsonValue			 V = UD->mJsonArray.at( i - 1 );

	return( lua_pushjsonvalue( L, V ) );
}

int lua_json::luaObjectPairs( lua_State *L )
{
	Q_UNUSED( L )

//	luaJsonObject		*UD = object( L, 1 );

	return( 0 );
}

int lua_json::luaObjectIsObject( lua_State *L )
{
	luaJsonObject		*UD = object( L, 1 );

	lua_pushboolean( L, UD->mIterator.value().isObject() );

	return( 1 );
}

int lua_json::luaObjectIsArray(lua_State *L)
{
	luaJsonObject		*UD = object( L, 1 );

	lua_pushboolean( L, UD->mIterator.value().isArray() );

	return( 1 );
}

int lua_json::luaDocumentGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaJsonDocument		*UD = document( L );

		if( !UD )
		{
			throw( mooException( E_TYPE, "invalid JSON document" ) );
		}

		if( UD->mJsonDocument.isArray() )
		{
			int					 i = luaL_checkinteger( L, 2 ) - 1;

			QJsonArray		 UA = UD->mJsonDocument.array();

			if( i < 0 || i >= UA.size() )
			{
				return( 0 );
			}

			return( lua_pushjsonvalue( L, UA.at( i ) ) );
		}

		if( UD->mJsonDocument.isObject() )
		{
			const char			*s = luaL_checkstring( L, 2 );

			QJsonObject			 UO = UD->mJsonDocument.object();

			return( lua_pushjsonvalue( L, UO.value( s ) ) );
		}

//		// Look for function in mLuaMap

//		lua_CFunction	 F;

//		if( ( F = mLuaDocumentMap.value( s, 0 ) ) != 0 )
//		{
//			lua_pushcfunction( L, F );

//			return( 1 );
//		}

//		// Nothing found

//		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
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

lua_json::luaJsonDocument * lua_json::document( lua_State *L, int pIndex )
{
	luaJsonDocument *H = (luaJsonDocument *)luaL_testudata( L, pIndex, luaJsonDocument::mLuaName );

	if( !H )
	{
		throw( mooException( E_TYPE, QString( "'JSON document' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

lua_json::luaJsonArray * lua_json::array( lua_State *L, int pIndex )
{
	luaJsonArray *H = (luaJsonArray *)luaL_testudata( L, pIndex, luaJsonArray::mLuaName );

	if( !H )
	{
		throw( mooException( E_TYPE, QString( "'JSON array' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

lua_json::luaJsonObject * lua_json::object( lua_State *L, int pIndex )
{
	luaJsonObject *H = (luaJsonObject *)luaL_testudata( L, pIndex, luaJsonObject::mLuaName );

	if( !H )
	{
		throw( mooException( E_TYPE, QString( "'JSON object' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}
