#ifndef TST_OBJECT_H
#define TST_OBJECT_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class TestObject : public LuaTestObject
{
	Q_OBJECT

private slots:
	void objectDefaults( void );
	void objectGetSet( void );
	void objectTree( void );
	void objectLocation( void );
	void objectVerbs( void );
	void objectProps( void );
	void objectPropsInherit( void );
};

#endif // TST_OBJECT_H
