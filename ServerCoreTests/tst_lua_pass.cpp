#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

// call moo.pass() from one verb to its parent

void ServerTest::luaPass1( void )
{
	LuaTestData			 TD;

	Object				*O1 = TD.OM.newObject();	// #4
	Object				*O2 = TD.OM.newObject();	// #5

	O1->setParent( OBJECT_ROOT );
	O2->setParent( O1->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "moo.pass()" ) );

	O2->verbAdd( "test", V2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( O1->id() );

	O1->propAdd( "result", P );

	// Call test

	TD.execute( QString( ";o( %1 ):test()" ).arg( O2->id() ) );

	// Check result

	Property	*R = O1->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.
}

// moo.pass() on two parents

void ServerTest::luaPass2( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	Object				*O3 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O3->id() );

	O1->setParent( 2 );
	O2->setParent( O1->id() );
	O3->setParent( O2->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );
	O3->setOwner( O3->id() );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "moo.pass()" ) );

	O2->verbAdd( "test", V2 );

	// Add verb

	Verb			 V3;

	V3.initialise();

	V3.setOwner( O3->id() );
	V3.setScript( QString( "moo.pass()" ) );

	O3->verbAdd( "test", V3 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( O1->id() );

	O1->propAdd( "result", P );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O1->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.
}

// moo.pass() on three parents (where the verb is not defined on the 4th child)

void ServerTest::luaPass3( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();	// #4
	Object				*O2 = TD.OM.newObject();	// #5
	Object				*O3 = TD.OM.newObject();	// #6
	Object				*O4 = TD.OM.newObject();	// #7

	O1->setParent( OBJECT_ROOT );
	O2->setParent( O1->id() );
	O3->setParent( O2->id() );
	O4->setParent( O3->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );
	O3->setOwner( O3->id() );
	O4->setOwner( O4->id() );

	// Add property to hold result

	if( true )
	{
		Property		P;

		P.initialise();

		P.setValue( "result" );
		P.setOwner( O1->id() );

		O1->propAdd( "result", P );
	}

	if( true )
	{
		Property		P;

		P.initialise();

		P.setValue( 0.0 );
		P.setOwner( O1->id() );
		P.setWrite( true );

		O1->propAdd( "count", P );
	}

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).count = o( %1 ).count + 1\n\no( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "o( %1 ).count = o( %1 ).count + 1\n\nmoo.pass()" ).arg( O1->id() ) );

	O2->verbAdd( "test", V2 );

	// Add verb

	Verb			 V3;

	V3.initialise();

	V3.setOwner( O3->id() );
	V3.setScript( QString( "o( %1 ).count = o( %1 ).count + 1\n\nmoo.pass()" ).arg( O1->id() ) );

	O3->verbAdd( "test", V3 );

	// Call test

	TD.execute( QString( ";o( %1 ):test()" ).arg( O4->id() ) );

	// Check result

	Property	*R1 = O1->prop( "result" );

	QVERIFY( R1 );
	QVERIFY( R1->type() == QVariant::String );
	QCOMPARE( R1->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.

	Property	*R2 = O1->prop( "count" );

	QVERIFY( R2 );
	QVERIFY( R2->type() == QVariant::Double );
	QCOMPARE( R2->value().toDouble(), 3 ); // The initial value of this in the called verb is the same as in the calling verb.
}

// moo.pass() on two parents (where the verb is not defined on the 2nd child)

void ServerTest::luaPass4( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	Object				*O3 = TD.OM.newObject();

	O1->setParent( OBJECT_ROOT );
	O2->setParent( O1->id() );
	O3->setParent( O2->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );
	O3->setOwner( O3->id() );

	// Add property to hold result

	if( true )
	{
		Property		P;

		P.initialise();

		P.setValue( "result" );
		P.setOwner( O1->id() );

		O1->propAdd( "result", P );
	}

	if( true )
	{
		Property		P;

		P.initialise();

		P.setValue( 0.0 );
		P.setOwner( O1->id() );
		P.setWrite( true );

		O1->propAdd( "count", P );
	}

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).count = o( %1 ).count + 1\n\no( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

//	Verb			 V2;

//	V2.initialise();

//	V2.setOwner( O2->id() );
//	V2.setScript( QString( "moo.pass()" ) );

//	O2->verbAdd( "test", V2 );

	// Add verb

	Verb			 V3;

	V3.initialise();

	V3.setOwner( O3->id() );
	V3.setScript( QString( "o( %1 ).count = o( %1 ).count + 1\n\nmoo.pass()" ).arg( O1->id() ) );

	O3->verbAdd( "test", V3 );

	// Call test

	TD.execute( QString( ";o( %1 ):test()" ).arg( O3->id() ) );

	// Check result

	Property	*R1 = O1->prop( "result" );

	QVERIFY( R1 );
	QVERIFY( R1->type() == QVariant::String );
	QCOMPARE( R1->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.

	Property	*R2 = O1->prop( "count" );

	QVERIFY( R2 );
	QVERIFY( R2->type() == QVariant::Double );
	QCOMPARE( R2->value().toDouble(), 2 ); // The initial value of this in the called verb is the same as in the calling verb.
}

// moo.pass() on three parents (where the verb is not defined on the 3rd child)

void ServerTest::luaPass5( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	Object				*O3 = TD.OM.newObject();
	Object				*O4 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O4->id() );

	O1->setParent( 2 );
	O2->setParent( O1->id() );
	O3->setParent( O2->id() );
	O4->setParent( O3->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );
	O3->setOwner( O3->id() );
	O4->setOwner( O4->id() );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "moo.pass()" ) );

	O2->verbAdd( "test", V2 );

	// Add verb

	Verb			 V4;

	V4.initialise();

	V4.setOwner( O4->id() );
	V4.setScript( QString( "moo.pass()" ) );

	O4->verbAdd( "test", V4 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( O1->id() );

	O1->propAdd( "result", P );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O1->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.
}

// moo.pass() on one parent, calling second verb that also does a one parent moo.pass()

void ServerTest::luaPass6( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	Object				*O3 = TD.OM.newObject();
	Object				*O4 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O4->id() );

	O1->setParent( 2 );
	O2->setParent( O1->id() );
	O3->setParent( 2 );
	O4->setParent( O3->id() );

	O1->setOwner( O1->id() );
	O2->setOwner( O2->id() );
	O3->setOwner( O3->id() );
	O4->setOwner( O4->id() );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O1->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "moo.pass()" ) );

	O2->verbAdd( "test", V2 );

	// Add verb

	Verb			 V3;

	V3.initialise();

	V3.setOwner( O3->id() );
	V3.setScript( QString( "o( %1 ):test()" ).arg( O2->id() ) );

	O3->verbAdd( "test", V3 );

	// Add verb

	Verb			 V4;

	V4.initialise();

	V4.setOwner( O4->id() );
	V4.setScript( QString( "moo.pass()" ) );

	O4->verbAdd( "test", V4 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( O1->id() );

	O1->propAdd( "result", P );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O1->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) ); // The initial value of this in the called verb is the same as in the calling verb.
}
