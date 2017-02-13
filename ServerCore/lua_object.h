#ifndef LUA_OBJECT_H
#define LUA_OBJECT_H

#include <QMap>
#include "lua_utilities.h"
#include "object.h"

class lua_object
{
public:
	static ObjectId argId( lua_State *L, int pIndex = 1 );
	static Object *argObj( lua_State *L, int pIndex = 1 );
	static int lua_pushobject( lua_State *L, Object *O );
	static int lua_pushobjectid( lua_State *L, ObjectId I );

	typedef struct luaHandle
	{
		luaHandle( void )
		{
			O = OBJECT_NONE;
		}

		luaHandle( const luaHandle &H )
		{
			O = H.O;
		}

		~luaHandle( void )
		{

		}

		ObjectId		O;

		static const char		*mLuaName;
	} luaHandle;

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	// LUA Commands

	static int luaGC( lua_State *L );
	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );
	static int luaToString( lua_State *L );

	static int luaEQ( lua_State *L );

	static int luaCreate( lua_State *L );
	static int luaObject( lua_State *L );

	static int luaRecycle( lua_State *L );

	static int luaChildren( lua_State *L );
	static int luaChild( lua_State *L );

	static int luaPlayers( lua_State *L );

	static int luaVerb( lua_State *L );
	static int luaProperty( lua_State *L );

	static int luaAliasAdd( lua_State *L );
	static int luaAliasDel( lua_State *L );

	static int luaVerbAdd( lua_State *L );
	static int luaVerbDel( lua_State *L );
	static int luaVerbCall( lua_State *L );

	static int luaPropAdd( lua_State *L );
	static int luaPropDel( lua_State *L );
	static int luaPropClear( lua_State *L );

	static int luaNotify( lua_State *L );

	static int luaProps( lua_State *L );
	static int luaVerbs( lua_State *L );

	static int luaFind( lua_State *L );

	static int luaPushVariant( lua_State *L, const QVariant &pV );

	static LuaMap				 mLuaMap;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
	static const luaL_Reg		 mLuaInstanceFunctions[];

	friend class lua_moo;
	friend class ServerTest;
};

Q_DECLARE_METATYPE( lua_object::luaHandle );

#endif // LUA_OBJECT_H
