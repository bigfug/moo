#ifndef TST_LUA_CALL_VALUES_H
#define TST_LUA_CALL_VALUES_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class TestLuaCallValues : public LuaTestObject
{
	Q_OBJECT

private slots:
	void luaCallValueFirst_data( void );
	void luaCallValueFirst( void );

	void luaCallValueSecond_data( void );
	void luaCallValueSecond( void );

	void luaCallValueThird_data( void );
	void luaCallValueThird( void );

	void luaCallValueInherit( void );
};

#endif // TST_LUA_CALL_VALUES_H
