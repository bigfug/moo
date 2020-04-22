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

	void luaCallValueObjectFirst( void );
	void luaCallValuePlayerFirst( void );
	void luaCallValueCallerFirst( void );
	void luaCallValueProgrammerFirst( void );

	void luaCallValueObjectSecond( void );
	void luaCallValuePlayerSecond( void );
	void luaCallValueCallerSecond( void );
	void luaCallValueProgrammerSecond( void );

	void luaCallValueObjectThird( void );
	void luaCallValuePlayerThird( void );
	void luaCallValueCallerThird( void );
	void luaCallValueProgrammerThird( void );

	void luaCallValueObjectFirstWizard( void );
	void luaCallValuePlayerFirstWizard( void );
	void luaCallValueCallerFirstWizard( void );
	void luaCallValueProgrammerFirstWizard( void );

	void luaCallValueObjectSecondWizard( void );
	void luaCallValuePlayerSecondWizard( void );
	void luaCallValueCallerSecondWizard( void );
	void luaCallValueProgrammerSecondWizard( void );

	void luaCallValueObjectThirdWizard( void );
	void luaCallValuePlayerThirdWizard( void );
	void luaCallValueCallerThirdWizard( void );
	void luaCallValueProgrammerThirdWizard( void );

	void luaPass1( void );
	void luaPass2( void );
	void luaPass3( void );
	void luaPass4( void );
	void luaPass5( void );
	void luaPass6( void );

	void luaPropAddSecurityPass( void );
	void luaPropAddSecurityFail( void );
	void luaPropAddSecurityWizardOwner( void );
	void luaPropAddSecurityWizard( void );

	void luaPropDelSecurityPass( void );

	void luaPropInheritance( void );

	void luaVerbVerbCall( void );
	void luaVerbParentVerbCall( void );

	void taskRollbackObjectCreate( void );
	void taskRollbackObjectRecycle( void );

	void taskRollbackObjectRead( void );
	void taskRollbackObjectWrite( void );
	void taskRollbackObjectFertile( void );
	void taskRollbackObjectName( void );
	void taskRollbackObjectOwner( void );
	void taskRollbackObjectParent( void );
	void taskRollbackObjectLocation( void );
	void taskRollbackObjectPlayer( void );
	void taskRollbackObjectProgrammer( void );
	void taskRollbackObjectWizard( void );

	void taskRollbackObjectPropAdd( void );
	void taskRollbackObjectPropDelete( void );
	void taskRollbackObjectPropClear( void );
	void taskRollbackObjectPropValue( void );
	void taskRollbackObjectPropInherit( void );

	void taskRollbackObjectAliasAdd();
	void taskRollbackObjectAliasDelete();
};

#endif // TST_SERVERTEST_H
