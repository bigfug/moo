#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_prop.h"
#include "lua_task.h"

void ServerTest::luaPropNumber( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();

	O->setOwner( Programmer->id() );

	// NUMBERS

	if( true )
	{
		QString			 CMD = QString( "o( %1 ):propadd( 'test', 14 )" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->object(), O->id() );
		QCOMPARE( P->name(), QStringLiteral( "test" ) );
		QCOMPARE( P->parent(), OBJECT_NONE );
		QCOMPARE( P->owner(), Programmer->id() );
		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 14.0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=20" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QCOMPARE( P->type(), QVariant::Double );
		QCOMPARE( P->value().toDouble(), 20.0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=o( %1 ).test + 20" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::Double );
		QVERIFY( P->value().toDouble() == 40.0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test='bad string'" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::Double );
		QVERIFY( P->value().toDouble() == 40.0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propdel( 'test' )" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P == 0 );
	}

	ObjectManager::reset();
}

void ServerTest::luaPropString( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();

	O->setOwner( Programmer->id() );

	// STRINGS

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', 'blah' );" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QVERIFY( P->owner() == Programmer->id() );
		QVERIFY( P->type() == QVariant::String );
		QVERIFY( P->value().toString().compare( "blah" ) == 0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test='blah2'" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::String );
		QVERIFY( P->value().toString().compare( "blah2" ) == 0 );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=o( %1 ).test .. 'blah'" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::String );
		QVERIFY( P->value().toString().compare( "blah2blah" ) == 0 );
	}

//	if( true )
//	{
//		lua_task		 Com( &Con, QString( ";o( %1 ).test=20;" ).arg( O->id() ) );

//		Com.eval();

//		Property	*P = O->prop( "test" );

//		QVERIFY( P->type() == QVariant::String );
//		QCOMPARE( P->value().toString(), QString( "blah2blah" ) );
//	}

	if( true )
	{
		QString			 CMD = QString( "o( %1 ):propdel( 'test' )" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Property	*P = O->prop( "test" );

		QVERIFY( P == 0 );

	}

	ObjectManager::reset();
}

void ServerTest::luaPropBoolean( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();

	O->setOwner( Programmer->id() );

	// BOOL

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', true );" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QVERIFY( P->owner() == Programmer->id() );
		QVERIFY( P->type() == QVariant::Bool );
		QVERIFY( P->value().toBool() == true );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=false;" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::Bool );
		QVERIFY( P->value().toBool() == false );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=20;" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P->type() == QVariant::Bool );
		QVERIFY( P->value().toBool() == false );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propdel( 'test' )" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P == 0 );
	}

	ObjectManager::reset();
}

void ServerTest::luaPropObject( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();

	O->setOwner( Programmer->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', o( %2 ) );" ).arg( O->id() ).arg( O1->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->owner(), Programmer->id() );
		QCOMPARE( P->value().typeName(), "lua_object::luaHandle" );
		QCOMPARE( P->value().value<lua_object::luaHandle>().O, O1->id() );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=o( %2 );" ).arg( O->id() ).arg( O2->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QCOMPARE( P->value().typeName(), "lua_object::luaHandle" );
		QCOMPARE( P->value().value<lua_object::luaHandle>().O, O2->id() );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=20;" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QCOMPARE( P->value().typeName(), "lua_object::luaHandle" );
		QCOMPARE( P->value().value<lua_object::luaHandle>().O, O2->id() );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propdel( 'test' )" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P == 0 );
	}

	ObjectManager::reset();
}

void ServerTest::luaPropList( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();

	O->setOwner( Programmer->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', { 12.0, 'thirteen', { 14, 'fifteen' } } );" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->owner(), Programmer->id() );
		QCOMPARE( P->type(), QVariant::Map );

		QVariantMap		VarMap = P->value().toMap();

		QCOMPARE( VarMap.size(), 3 );
		QCOMPARE( VarMap.value( "1" ).type(), QVariant::Double );
		QCOMPARE( VarMap.value( "1" ).toDouble(), 12.0 );
		QCOMPARE( VarMap.value( "2" ).type(), QVariant::String );
		QCOMPARE( VarMap.value( "3" ).type(), QVariant::Map );
		QCOMPARE( VarMap.value( "3" ).toMap().size(), 2 );
		QCOMPARE( VarMap.value( "3" ).toMap().value( "1" ).type(), QVariant::Double );
		QCOMPARE( VarMap.value( "3" ).toMap().value( "1" ).toDouble(), 14.0 );
		QCOMPARE( VarMap.value( "3" ).toMap().value( "2" ).type(), QVariant::String );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test={ 'hello', 'world' };" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QCOMPARE( P->type(), QVariant::Map );
		QCOMPARE( P->value().toMap().size(), 2 );
		QCOMPARE( P->value().toMap().value( "1" ).type(), QVariant::String );
		QCOMPARE( P->value().toMap().value( "1" ).toString(), QString( "hello" ) );
		QCOMPARE( P->value().toMap().value( "2" ).type(), QVariant::String );
		QCOMPARE( P->value().toMap().value( "2" ).toString(), QString( "world" ) );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ).test=20;" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QCOMPARE( P->type(), QVariant::Map );
		QCOMPARE( P->value().toMap().size(), 2 );
		QCOMPARE( P->value().toMap().value( "1" ).type(), QVariant::String );
		QCOMPARE( P->value().toMap().value( "1" ).toString(), QString( "hello" ) );
		QCOMPARE( P->value().toMap().value( "2" ).type(), QVariant::String );
		QCOMPARE( P->value().toMap().value( "2" ).toString(), QString( "world" ) );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propdel( 'test' )" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P == 0 );
	}

	ObjectManager::reset();
}

void ServerTest::luaPropGetSet( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*O = OM.newObject();

	O->setOwner( Programmer->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ):propadd( 'test', 14 )" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QVERIFY( P->owner() == Programmer->id() );
		QVERIFY( P->type() == QVariant::Double );
		QVERIFY( P->value().toDouble() == 14.0 );
		QCOMPARE( P->read(), false );
		QCOMPARE( P->write(), false );
		QCOMPARE( P->change(), true );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):prop( 'test' ).r = true" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->read(), true );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):prop( 'test' ).c = true" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->change(), true );
	}

	if( true )
	{
		lua_task::process( QString( "o( %1 ):prop( 'test' ).w = true" ).arg( O->id() ), CID, Programmer->id() );

		Property	*P = O->prop( "test" );

		QVERIFY( P != 0 );
		QCOMPARE( P->write(), true );
	}

	ObjectManager::reset();
}

void ServerTest::luaPropGetIndex( void )
{
	LuaTestData		TD;

	Object			*O1 = TD.OM.newObject();

	O1->setOwner( TD.programmerId() );

	TD.process( QString( "o( %1 ):propadd( 'result', 'no result' )" ).arg( O1->id() ) );

	Property		*P1 = O1->prop( "result" );

	QVERIFY( P1 );

	TD.process( QString( "o( %1 ):propadd( 'test', { k1 = { 'v1', 'v2' } } )" ).arg( O1->id() ) );

	QVERIFY( O1->prop( "test" ) );

	TD.process( QString( "o( %1 ).result = o( %1 ).test.k1[ 1 ]" ).arg( O1->id() ) );

	QCOMPARE( P1->value().toString(), QString( "v1" ) );

	TD.process( QString( "o( %1 ).result = o( %1 ).test.k1[ 2 ]" ).arg( O1->id() ) );

	QCOMPARE( P1->value().toString(), QString( "v2" ) );
}

void ServerTest::luaPropSetIndex( void )
{
	LuaTestData		TD;

	Object			*O1 = TD.OM.newObject();

	O1->setOwner( TD.programmerId() );

	TD.process( QString( "o( %1 ):propadd( 'result', 'no result' )" ).arg( O1->id() ) );

	Property		*P1 = O1->prop( "result" );

	QVERIFY( P1 );

	TD.process( QString( "o( %1 ):propadd( 'test', { k1 = { 'v1', k2 = { 'v2', 'v3' } } } )" ).arg( O1->id() ) );

	QVERIFY( O1->prop( "test" ) );

	TD.process( QString( "o( %1 ).test.k1.k2:set( 2, 'v4' )" ).arg( O1->id() ) );

	TD.process( QString( "o( %1 ).result = o( %1 ).test.k1.k2[ 2 ]" ).arg( O1->id() ) );

	QCOMPARE( P1->value().toString(), QString( "v4" ) );

	TD.process( QString( "o( %1 ).test.k1.k2:set( 3, 'v5' )" ).arg( O1->id() ) );

	TD.process( QString( "o( %1 ).result = o( %1 ).test.k1.k2[ 3 ]" ).arg( O1->id() ) );

	QCOMPARE( P1->value().toString(), QString( "v5" ) );

	TD.process( QString( "o( %1 ).test.k1.k2:clear( 3 )" ).arg( O1->id() ) );

	lua_task	T = TD.eval( QString( "o( %1 ).result = o( %1 ).test.k1.k2[ 3 ]" ).arg( O1->id() ) );

	QVERIFY( T.error() );
}
