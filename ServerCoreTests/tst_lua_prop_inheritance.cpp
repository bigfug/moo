#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_prop.h"
#include "lua_task.h"

#include "luatestdata.h"

void ServerTest::luaPropInheritance_data()
{
	QTest::addColumn<bool>( "pChange" );
	QTest::addColumn<ObjectId>( "pOwnerId" );

	QTest::newRow( "change" )	<< true  << 6;
	QTest::newRow( "!change" )	<< false << 3;
}

void ServerTest::luaPropInheritance( void )
{
	QFETCH( bool, pChange );
	QFETCH( ObjectId, pOwnerId );

	LuaTestData		TD;

	Object			*O1 = TD.OM.newObject();
	Object			*O2 = TD.OM.newObject();
	Object			*O3 = TD.OM.newObject();

	O1->setOwner( TD.programmerId() );
	O2->setOwner( TD.programmerId() );
	O3->setOwner( TD.programmerId() );

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );

	if( true )
	{
		TD.process( QString( "o( %1 ):propadd( 'test', 14 )" ).arg( O2->id() ) );

		Property	*P = O2->prop( "test" );

		QVERIFY( P != 0 );

		P->setChange( pChange );

		QCOMPARE( P->object(), O2->id() );
		QCOMPARE( P->name(), QStringLiteral( "test" ) );
		QCOMPARE( P->parent(), OBJECT_NONE );
		QCOMPARE( P->owner(), TD.programmerId() );
		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 14.0 );
	}

	// We shouldn't be able to add a property to an object if the ancestors already contains it

	if( true )
	{
		TD.process( QString( "o( %1 ):propadd( 'test', 30 )" ).arg( O3->id() ) );

		QVERIFY( !O3->prop( "test" ) );
	}

	// We shouldn't be able to add a property to an object if the descendants already contains it

	if( true )
	{
		TD.process( QString( "o( %1 ):propadd( 'test', 30 )" ).arg( O1->id() ) );

		QVERIFY( !O1->prop( "test" ) );
	}

	// Test the inheritance

	if( true )
	{
		TD.process( QString( "o( %1 ).test = 20" ).arg( O3->id() ) );

		Property	*P = O3->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->object(), O3->id() );
		QCOMPARE( P->name(), QStringLiteral( "test" ) );
		QCOMPARE( P->parent(), O2->id() );
		QCOMPARE( P->owner(), pOwnerId );
		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 20.0 );
	}

	if( true )
	{
		TD.process( QString( "o( %1 ):propclr( 'test' )" ).arg( O3->id() ) );

		Property	*P = O3->prop( "test" );

		QVERIFY( !P );
	}
}
