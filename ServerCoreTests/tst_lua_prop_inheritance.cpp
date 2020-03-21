#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_prop.h"
#include "lua_task.h"

void ServerTest::luaPropInheritance( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();

	O1->setOwner( Programmer->id() );
	O2->setOwner( Programmer->id() );
	O3->setOwner( Programmer->id() );

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', 14 )" ).arg( O2->id() ), CID, Programmer->id() );

		Property	*P = O2->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->object(), O2->id() );
		QCOMPARE( P->name(), QStringLiteral( "test" ) );
		QCOMPARE( P->parent(), OBJECT_NONE );
		QCOMPARE( P->owner(), Programmer->id() );
		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 14.0 );
	}

	// We shouldn't be able to add a property to an object if the ancestors already contains it

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', 30 )" ).arg( O3->id() ), CID, Programmer->id() );

		Property	*P = O3->prop( "test" );

		QVERIFY( P == 0 );
	}

	// We shouldn't be able to add a property to an object if the descendants already contains it

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', 30 )" ).arg( O1->id() ), CID, Programmer->id() );

		Property	*P = O1->prop( "test" );

		QVERIFY( P == 0 );
	}

	// Test the inheritance

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test = 20" ).arg( O3->id() ), CID, Programmer->id() );

		Property	*P = O3->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->object(), O3->id() );
		QCOMPARE( P->name(), QStringLiteral( "test" ) );
		QCOMPARE( P->parent(), O2->id() );
		QCOMPARE( P->owner(), O3->id() );
		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 20.0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propclr( 'test' )" ).arg( O3->id() ), CID, Programmer->id() );

		Property	*P = O3->prop( "test" );

		QVERIFY( P == 0 );
	}
}
