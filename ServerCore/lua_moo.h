#ifndef LUA_MOO_H
#define LUA_MOO_H

#include <cstdio>
#include <iostream>

#include <QVector>

#include "mooglobal.h"
#include "lua_utilities.h"

class lua_moo
{
private:
	static const luaL_Reg				mLuaGlobal[];
	static const luaL_Reg				mLuaMeta[];
	static const luaL_Reg				mLuaStatic[];
	static const luaL_Reg				mLuaGetFunc[];

	static LuaMap						mLuaFun;
	static LuaMap						mLuaGet;
	static LuaMap						mLuaSet;

	static void luaRegisterAllStates( lua_State *L );

	static void luaRegisterState( lua_State *L );

	static int luaGlobalIndex( lua_State *L );

	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );

	static int luaNotify( lua_State *L );
	static int luaRoot( lua_State *L );
	static int luaPass( lua_State *L );
	static int luaEval( lua_State *L );
	static int luaDebug( lua_State *L );
	static int luaPlayers( lua_State *L );

	static int luaLastObject( lua_State *L );

	static int luaPanic( lua_State *L );

	static void typeDump( lua_State *L, const int i )
	{
		int t = lua_type(L, i);

		switch (t)
		{
			case LUA_TSTRING:  /* strings */
				printf( "%d: `%s'", i, lua_tostring(L, i));
				break;

			case LUA_TBOOLEAN:  /* booleans */
				printf( "%d: %s", i, lua_toboolean(L, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:  /* numbers */
				printf("%d: %g", i, lua_tonumber(L, i));
				break;

			case LUA_TTABLE:
				tableDump( L, i );
				break;

			default:  /* other values */
				printf("%d: %s", i, lua_typename(L, t));
				break;

		}
	}

	static void tableDump( lua_State *L, const int t )
	{
		//printf( "enter: %d\n", lua_gettop( L ) );

		printf( "table\n{\n" );

		lua_pushnil( L );		// first key

		while( lua_next( L, t ) != 0 )
		{
			typeDump( L, -2 );

			printf( "%s", lua_typename( L, lua_type( L, -1 ) ) );

			//typeDump( L, -1 );

			printf( "\n" );

			/* removes 'value'; keeps 'key' for next iteration */

			lua_pop( L, 1 );
		}

		printf( "}\n" );

		//printf( "exit: %d\n", lua_gettop( L ) );
	}

	static void initialise( void );

public:
	static void initialiseAll( void );

	static void luaNewState( lua_State *L );

	static void luaSetEnv( lua_State *L );

	static void stackDump (lua_State *L)
	{
		int i;
		int top = lua_gettop(L);

		for (i = 1; i <= top; i++) /* repeat for each level */
		{
			typeDump( L, i );

			printf("  ");  /* put a separator */
		}

		printf("\n");  /* end the listing */

		std::cout.flush();
	}

	static void addFunctions( const luaL_Reg *pFuncs );
	static void addGet( const luaL_Reg *pFuncs );
	static void addSet( const luaL_Reg *pFuncs );
};

#endif // LUA_MOO_H
