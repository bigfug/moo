#ifndef LUA_PROP_H
#define LUA_PROP_H

#include "mooglobal.h"
#include "lua_utilities.h"

class Property;
class Object;

class lua_prop
{
public:
	typedef struct luaProp
	{
		QString			*mName;
		ObjectId		 mObjectId;
		Property		*mProperty;
	} luaProp;

	static void lua_pushproperty( lua_State *L, Property *pProperty );

	static void lua_pushproperty( lua_State *L, ObjectId pObjectId, const QString &pName, Property *pProperty );

	static luaProp *arg( lua_State *L, int pIndex = 1 );

	static int luaSetValue( lua_State *L, const QString &pPrpNam, Object *pObject, Property *pFndPrp, Object *pFndObj, int pIdx );

	static void luaNewRecurse( lua_State *L, int pIdx, QVariant &pVariant  );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaGC( lua_State *L );

	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );

	static int luaDump( lua_State *L );
	static int luaProgram( lua_State *L );

	static const char			*mLuaName;

	static LuaMap				 mLuaMap;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
	static const luaL_Reg		 mLuaInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_PROP_H
