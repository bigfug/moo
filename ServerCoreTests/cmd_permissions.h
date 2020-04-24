#ifndef CMD_PERMISSIONS_H
#define CMD_PERMISSIONS_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class cmd_permissions : public LuaTestObject
{
	Q_OBJECT

private slots:
	void testArgs_data( void );
	void testArgs( void );

	void testArgs2_data( void );
	void testArgs2( void );

	void testArgs3_data( void );
	void testArgs3( void );
};

#endif // CMD_PERMISSIONS_H
