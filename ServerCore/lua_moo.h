#ifndef LUA_MOO_H
#define LUA_MOO_H

#include <QSettings>

#include <cstdio>
#include <iostream>

#include <QVector>
#include <QNetworkAccessManager>

#include "mooglobal.h"
#include "lua_utilities.h"

#define MOO_SETTINGS	lua_moo::settingsFilePath(), QSettings::IniFormat

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

	static QNetworkAccessManager		mNAM;

	static QString						mSettingsFilePath;

	static void luaRegisterAllStates( lua_State *L );

	static void luaRegisterState( lua_State *L );

	static int luaGlobalIndex( lua_State *L );

	static int luaGet( lua_State *L );
	static int luaSet( lua_State *L );

	static int luaBroadcast( lua_State *L );
	static int luaNotify( lua_State *L );
	static int luaRoot( lua_State *L );
	static int luaSystem( lua_State *L );
	static int luaPass( lua_State *L );
	static int luaEval( lua_State *L );
	static int luaElevate( lua_State *L );
	static int luaHash( lua_State *L );
	static int luaDebug( lua_State *L );
	static int luaFindPlayer( lua_State *L );
	static int luaFindByProp( lua_State *L );
	static int luaRead( lua_State *L );
	static int luaFind( lua_State *L );
	static int luaIsValidObject( lua_State *L );

	static int luaSetCookie( lua_State *L );
	static int luaCookie( lua_State *L );
	static int luaClearCookie( lua_State *L );

	static int luaTimestamp( lua_State *L );

	static int luaCheckPoint( lua_State *L );

	static int luaLastObject( lua_State *L );

	static int luaNetworkGet( lua_State *L );

	static int luaPanic( lua_State *L );

	static int luaGMCP( lua_State *L );

	static int luaImport( lua_State *L );

	static int luaConnect( lua_State *L );
	static int luaDisconnect( lua_State *L );

	static void typeDump( lua_State *L, const int i )
	{
		int t = lua_type(L, i);

		switch (t)
		{
			case LUA_TSTRING:  /* strings */
				std::cout << i << ": \"" << lua_tostring(L, i) << "\"";
				break;

			case LUA_TBOOLEAN:  /* booleans */
				std::cout << i << ": " << ( lua_toboolean( L, i ) ? "true" : "false" );
				break;

			case LUA_TNUMBER:  /* numbers */
				std::cout << i << ": " << lua_tonumber(L, i);
				break;

			case LUA_TTABLE:
				tableDump( L, i );
				break;

			default:  /* other values */
				std::cout << i << ": " << lua_typename(L, t);
				break;

		}
	}

	static void tableDump( lua_State *L, const int t )
	{
		//printf( "enter: %d\n", lua_gettop( L ) );

		std::cout << t << ": table" << std::endl << "{" << std::endl;

		lua_pushvalue( L, t );

		lua_pushnil( L );		// first key

		while( lua_next( L, -2 ) != 0 )
		{
			typeDump( L, -2 );

			std::cout << " " << lua_typename( L, lua_type( L, -1 ) );

			//typeDump( L, -1 );

			std::cout << std::endl;

			/* removes 'value'; keeps 'key' for next iteration */

			lua_pop( L, 1 );
		}

		std::cout << "}" << std::endl;

		//printf( "exit: %d\n", lua_gettop( L ) );

		lua_pop( L, 1 );
	}

	static void initialise( void );

public:
	static void setSettingsFilePath( QString pFilePath );

	static QString settingsFilePath( void );

	static void initialiseAll( void );

	static void luaNewState( lua_State *L );

	static lua_State *luaNewState( void );

	static void stackDump (lua_State *L)
	{
		int i;
		int top = lua_gettop(L);

		for (i = 1; i <= top; i++) /* repeat for each level */
		{
			typeDump( L, i );

			std::cout << "  ";  /* put a separator */
		}

		std::cout << std::endl << std::endl;
	}

	static void stackReverseDump (lua_State *L)
	{
		std::cout << "stackReverseDump:" << std::endl;

		int i;
		int top = lua_gettop(L);

		for (i = 1; i <= top; i++) /* repeat for each level */
		{
			typeDump( L, -i );

			std::cout << "  ";  /* put a separator */
		}

		std::cout << std::endl << std::endl;
	}

	static void addFunctions( const luaL_Reg *pFuncs );
	static void addGet( const luaL_Reg *pFuncs );
	static void addSet( const luaL_Reg *pFuncs );

protected:
	static void luaSetEnv( lua_State *L );

	static QVariantMap parseReadArgs( lua_State *L, int pIndex );
};

#endif // LUA_MOO_H
