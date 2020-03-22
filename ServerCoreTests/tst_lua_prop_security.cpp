#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void ServerTest::luaPropAddSecurityPass( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() );

	O->setParent( TD.Programmer->id() );
	O->setOwner( TD.Programmer->id() );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	QVERIFY( R != 0 );
	QVERIFY( R->owner() == TD.Programmer->id() );
}

void ServerTest::luaPropAddSecurityFail( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() );

	O->setParent( 2 );
	O->setOwner( 2 );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	QVERIFY( R == 0 );
}

void ServerTest::luaPropAddSecurityWizardOwner( void )
{
	LuaTestData			 TD;
	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() );

	O->setParent( TD.Programmer->id() );
	O->setOwner( TD.Programmer->id() );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	QVERIFY( R != 0 );
	QVERIFY( R->owner() == TD.Programmer->id() );
}

void ServerTest::luaPropAddSecurityWizard( void )
{
	LuaTestData			 TD;
	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() );

	O->setParent( 2 );
	O->setOwner( 2 );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	QVERIFY( R != 0 );
	QVERIFY( R->owner() == TD.Programmer->id() );
}

//-----------------------------

void ServerTest::luaPropDelSecurityPass( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propdel( 'test' )" ).arg( O->id() );

	O->setParent( TD.Programmer->id() );
	O->setOwner( TD.Programmer->id() );

	Property			 P;

	P.initialise();

	P.setOwner( TD.Programmer->id() );

	O->propAdd( "test", P );

	QVERIFY( O->prop( "test" ) != 0 );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	QVERIFY( R == 0 );
}
