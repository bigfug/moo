#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void ServerTest::luaPropAddSecurity_data( void )
{
	QTest::addColumn<ObjectId>( "pOwner" );
	QTest::addColumn<bool>( "pWizard" );
	QTest::addColumn<bool>( "pObjectWrite" );
	QTest::addColumn<ObjectId>( "pResult" );
	QTest::addColumn<bool>( "pValid" );

	QTest::newRow( "owner" )			<< 3 << false << false << 3 << true;
	QTest::newRow( "not owner" )		<< 2 << false << false << 3 << false;
	QTest::newRow( "wizard owner" )		<< 3 << true  << false << 0 << true;
	QTest::newRow( "wizard not owner" )	<< 2 << true  << false << 0 << true;

	QTest::newRow( "owner (o:w)" )				<< 3 << false << true << 3 << true;
	QTest::newRow( "not owner (o:w)" )			<< 2 << false << true << 3 << true;
	QTest::newRow( "wizard owner (o:w)" )		<< 3 << true  << true << 0 << true;
	QTest::newRow( "wizard not owner (o:w)" )	<< 2 << true  << true << 0 << true;
}

void ServerTest::luaPropAddSecurity( void )
{
	QFETCH( ObjectId, pOwner );
	QFETCH( bool, pWizard );
	QFETCH( bool, pObjectWrite );
	QFETCH( ObjectId, pResult );
	QFETCH( bool, pValid );

	LuaTestData			 TD;

	Object				*O  = TD.OM.newObject();

	O->setOwner( pOwner );
	O->setWrite( pObjectWrite );

	// Call test

	TD.execute( QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() ), pWizard ? OBJECT_SYSTEM : TD.programmerId() );

	// Check result

	Property	*R = O->prop( "test" );

	if( pValid )
	{
		QVERIFY( R );
		QCOMPARE( R->owner(), pResult );
	}
	else
	{
		QVERIFY( !R );
	}
}

//-----------------------------

void ServerTest::luaPropDelSecurity_data( void )
{
	QTest::addColumn<ObjectId>( "pObjectOwner" );
	QTest::addColumn<bool>( "pWizard" );
	QTest::addColumn<ObjectId>( "pPropertyOwner" );
	QTest::addColumn<bool>( "pValid" );

	QTest::newRow( "owner" )			<< 3 << false << 3 << false;
	QTest::newRow( "not owner" )		<< 2 << false << 3 << true;
	QTest::newRow( "wizard owner" )		<< 3 << true  << 3 << false;
	QTest::newRow( "wizard not owner" )	<< 2 << true  << 3 << false;
}

void ServerTest::luaPropDelSecurity( void )
{
	QFETCH( ObjectId, pObjectOwner );
	QFETCH( bool, pWizard );
	QFETCH( ObjectId, pPropertyOwner );
	QFETCH( bool, pValid );

	LuaTestData			 TD;

	TD.Programmer->setWizard( pWizard );

	Object				*O  = TD.OM.newObject();

	O->setParent( pObjectOwner );
	O->setOwner( pObjectOwner );

	Property			 P;

	P.initialise();

	P.setOwner( pPropertyOwner );

	O->propAdd( "test", P );

	QVERIFY( O->prop( "test" ) );

	// Call test

	TD.execute( QString( ";o( %1 ):propdel( 'test' )" ).arg( O->id() ), pWizard ? OBJECT_SYSTEM : TD.programmerId() );

	// Check result

	Property	*R = O->prop( "test" );

	if( pValid )
	{
		QVERIFY( R );
	}
	else
	{
		QVERIFY( !R );
	}
}
