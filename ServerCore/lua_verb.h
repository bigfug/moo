#ifndef LUA_VERB_H
#define LUA_VERB_H

#include "mooglobal.h"

#include "lua_utilities.h"

class Verb;

class lua_verb
{
public:
	typedef struct luaVerb
	{
//		QString		*mName;
//		ObjectId	 mObjectId;
		Verb		*mVerb;
	} luaVerb;

	static void lua_pushverb(lua_State *L, Verb *V );

	static luaVerb *arg( lua_State *L, int pIndex = 1 );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaGC( lua_State *L );

	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );

	static int luaAliasAdd( lua_State *L );
	static int luaAliasRem( lua_State *L );

	static int luaDump( lua_State *L );
	static int luaProgram( lua_State *L );
	static int luaEdit( lua_State *L );

	static const char			*mLuaName;

	static LuaMap				 mLuaMap;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaInstance[];
	static const luaL_Reg		 mLuaInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_VERB_H
