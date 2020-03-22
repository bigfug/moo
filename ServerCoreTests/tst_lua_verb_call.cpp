#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_prop.h"
#include "lua_verb.h"
#include "lua_task.h"

#include "luatestdata.h"

void ServerTest::luaVerbVerbCall( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Verb				 V1;

	V1.initialise();

	V1.setDirectObjectArgument( NONE );
	V1.setPrepositionArgument( NONE );
	V1.setIndirectObjectArgument( NONE );

	V1.setOwner( 2 );

	V1.setScript( "return( tostring( moo.object.id ) )" );

	O1->verbAdd( "testverb1", V1 );

	Object				*O2 = TD.OM.newObject();
	Verb				 V2;

	V2.initialise();

	V2.setDirectObjectArgument( NONE );
	V2.setPrepositionArgument( NONE );
	V2.setIndirectObjectArgument( NONE );

	V2.setOwner( 2 );

	V2.setScript( QString( "moo.object.result = o( %1 ):testverb1()" ).arg( O1->id() ) );

	O2->verbAdd( "testverb2", V2 );

	QString				 CD = QString( ";o( %1 ):testverb2()" ).arg( O2->id() );

	TD.execute( CD );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( 2 );

	O2->propAdd( "result", P );

	// Call test

	TD.execute( CD );

//	qDebug() << "O1 =" << O1->id() << "O2 =" << O2->id();

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) );	// moo.eval sets object to -1
}

void ServerTest::luaVerbParentVerbCall( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Verb				 V1;

	V1.initialise();

	V1.setDirectObjectArgument( NONE );
	V1.setPrepositionArgument( NONE );
	V1.setIndirectObjectArgument( NONE );

	V1.setOwner( 2 );

	V1.setScript( "return( tostring( moo.object.id ) )" );

	O1->verbAdd( "testverb1", V1 );

	Object				*O2 = TD.OM.newObject();

	O2->setParent( O1->id() );

	Object				*O3 = TD.OM.newObject();
	Verb				 V3;

	V3.initialise();

	V3.setDirectObjectArgument( NONE );
	V3.setPrepositionArgument( NONE );
	V3.setIndirectObjectArgument( NONE );

	V3.setOwner( 2 );

	V3.setScript( QString( "moo.object.result = o( %1 ):testverb1()" ).arg( O2->id() ) );

	O3->verbAdd( "testverb2", V3 );

	QString				 CD = QString( ";o( %1 ):testverb2()" ).arg( O3->id() );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( 2 );

	O3->propAdd( "result", P );

	// Call test

	TD.execute( CD );

//	qDebug() << "O1 =" << O1->id() << "O2 =" << O2->id() << "O3 =" << O3->id();

	// Check result

	Property	*R = O3->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O2->id() ) );	// moo.eval sets object to -1
}
