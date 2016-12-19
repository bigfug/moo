#ifndef LUA_JSON_H
#define LUA_JSON_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <lua.hpp>

#include "lua_utilities.h"

class Connection;

class lua_json
{
public:
	typedef struct luaJsonDocument
	{
		QJsonDocument			 mJsonDocument;

		static const char		*mLuaName;

	} luaJsonDocument;

	typedef struct luaJsonArray
	{
		QJsonArray				 mJsonArray;
		QJsonArray::iterator	 mIterator;

		static const char		*mLuaName;

	} luaJsonArray;

	typedef struct luaJsonObject
	{
		QJsonObject				 mJsonObject;
		QJsonObject::iterator	 mIterator;

		static const char		*mLuaName;

	} luaJsonObject;

	static void lua_pushjsondocument( lua_State *L, const QJsonDocument &pJsonDocument );

	static void lua_pushjsonarray( lua_State *L, const QJsonArray &pJsonArray );

	static void lua_pushjsonobject( lua_State *L, const QJsonObject &pJsonObject );

	static int lua_pushjsonvalue( lua_State *L, const QJsonValue &pJsonValue );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaParse( lua_State *L );

	static int luaDocumentGet( lua_State *L );
	static int luaGetArray( lua_State *L );
	static int luaObjectGet( lua_State *L );

	static int luaDocumentIsObject( lua_State *L );
	static int luaDocumentIsArray( lua_State *L );

	static int luaDocumentObject( lua_State *L );
	static int luaDocumentArray( lua_State *L );

	static int luaArrayIPairs( lua_State *L );

	static int luaObjectPairs( lua_State *L );

	static int luaObjectIsObject( lua_State *L );
	static int luaObjectIsArray( lua_State *L );

	static luaJsonDocument *document( lua_State *L, int pIndex = 1 );
	static luaJsonArray *array( lua_State *L, int pIndex = 1 );
	static luaJsonObject *object( lua_State *L, int pIndex = 1 );

	static const luaL_Reg		 mLuaStatic[];

	static LuaMap				 mLuaDocumentMap;
	static LuaMap				 mLuaArrayMap;
	static LuaMap				 mLuaObjectMap;

	static const luaL_Reg		 mLuaArrayInstance[];
	static const luaL_Reg		 mLuaArrayInstanceFunctions[];

	static const luaL_Reg		 mLuaObjectInstance[];
	static const luaL_Reg		 mLuaObjectInstanceFunctions[];

	static const luaL_Reg		 mLuaDocumentInstance[];
	static const luaL_Reg		 mLuaDocumentInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_JSON_H
