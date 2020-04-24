#ifndef TST_SERVERTEST_H
#define TST_SERVERTEST_H

#include <QObject>
#include <QtTest/QtTest>

#include "mooglobal.h"

#include "luatestdata.h"

class Object;
class Connection;
class ObjectManager;

class ServerTest : public LuaTestObject
{
	Q_OBJECT

private slots:
	void taskDefaults( void );
	void taskGetSet( void );
	void taskSchedule( void );

	void propGetSet( void );
	void propInherit( void );

	void verbParse( void );
	void verbMatch( void );
	void verbGetSet( void );
	void verbArgs( void );

	void luaRegistration( void );

	void luaTaskTests( void );

	void luaMoveTestValidWhat( void );
	void luaMoveToRoot( void );
	void luaMove( void );

	void luaParentTestValidObject( void );
	void luaParentTestValidParent( void );
	void luaParentBasic( void );
	void luaParentBasicReparent( void );
	void luaParentLoopTest( void );
	void luaParentTestIsParentOf( void );
	void luaParentTestIsChildOf( void );

	void luaParentChangePropTest1( void );
	void luaParentChangePropTest2( void );
	void luaParentChangePropTest3( void );
	void luaParentChangePropTest4( void );

	void luaCreate( void );
	void luaRecycle( void );

	void luaPropNumber( void );
	void luaPropString( void );
	void luaPropBoolean( void );
	void luaPropObject( void );
	void luaPropList( void );

	void luaPropGetSet( void );

	void luaVerbAdd( void );
	void luaVerbDel( void );

	void luaPass1( void );
	void luaPass2( void );
	void luaPass3( void );
	void luaPass4( void );
	void luaPass5( void );
	void luaPass6( void );

	void luaPropAddSecurity_data( void );
	void luaPropAddSecurity( void );

	void luaPropDelSecurity_data( void );
	void luaPropDelSecurity( void );

	void luaPropInheritance_data( void );
	void luaPropInheritance( void );

	void luaVerbVerbCall( void );
	void luaVerbParentVerbCall( void );
};

#endif // TST_SERVERTEST_H
