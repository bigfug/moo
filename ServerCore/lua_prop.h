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

	static void luaNewRecurse( lua_State *L, int pIdx, QVariant &pVariant  );

	typedef QStringList					luaPropIndexPath;

	typedef struct luaPropIndex
	{
		Property				*mProperty;
		luaPropIndexPath		*mIndex;
	} luaPropIndex;

	static int lua_pushpropindex( lua_State *L, Property *pProperty );

	static int lua_pushpropindex( lua_State *L, Property *pProperty, const luaPropIndexPath &pPath );

	static luaPropIndex *propindex( lua_State *L, int pIndex = 1 );

private:
	typedef enum Fields
	{
		UNKNOWN,
		NAME,
		OWNER,
		READ,
		WRITE,
		CHANGE,
	} Fields;

	static const QMap<QString,Fields>		mFieldMap;

	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaGC( lua_State *L );

	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );

	static int luaDump( lua_State *L );
	static int luaProgram( lua_State *L );
	static int luaEdit( lua_State *L );
	static int luaValue( lua_State *L );

	static const char			*mLuaName;
	static const char			*mLuaPropIndexName;

	static LuaMap				 mLuaMap;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
	static const luaL_Reg		 mLuaInstanceFunctions[];

	static int luaPropIndexGC( lua_State *L );

	static int luaPropIndexGet( lua_State *L );
	static int luaPropIndexSet( lua_State *L );
	static int luaPropIndexClear( lua_State *L );
	static int luaPropIndexValue( lua_State *L );

	static LuaMap				 mLuaPropIndexMap;

	static const luaL_Reg		 mLuaPropIndexInstance[];
	static const luaL_Reg		 mLuaPropIndexInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_PROP_H
