#ifndef LUA_SERIALPORT_H
#define LUA_SERIALPORT_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <QSerialPort>
#include <QMap>

#include "lua_utilities.h"
#include "object.h"

class lua_serialport
{
public:
	typedef struct luaSerialPort
	{
		int					 mSerialId;

	} luaSerialPort;

	static void lua_pushserialport( lua_State *L, int pSerialId );

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static luaSerialPort *arg( lua_State *L, int pIndex = 1 );

	static int luaOpenSerialPort( lua_State *L );

	static int luaPort( lua_State *L );

	static int luaSetBaudRate( lua_State *L );

	static int luaGC( lua_State *L );

	static int luaGet( lua_State *L );

	static int luaOpen( lua_State *L );
	static int luaClose( lua_State *L );
	static int luaRead( lua_State *L );
	static int luaWrite( lua_State *L );

	static int luaRegister( lua_State *L );

	static const char					*mLuaName;

	static LuaMap						 mLuaMap;

	static const luaL_Reg				 mLuaStatic[];
	static const luaL_Reg				 mLuaInstance[];
	static const luaL_Reg				 mLuaInstanceFunctions[];

	static QMap<int,QSerialPort *>		 mSerialPortMap;

	friend class lua_moo;
};

#endif // LUA_SERIALPORT_H
