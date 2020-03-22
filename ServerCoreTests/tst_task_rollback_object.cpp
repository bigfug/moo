#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void ServerTest::taskRollbackObject( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*Owner      = OM.newObject();

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		QString			 CMD = QString( "moo.create()" );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		Com.rollback();

		QVERIFY( O->recycle() );

		OM.recycleObjects();
	}

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		lua_task::process( QString( "moo.create()" ), CID, Programmer->id() );

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QString			 CMD = QString( "o( %1 ):recycle()" ).arg( oid );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		QVERIFY( O->recycle() );

		QCOMPARE( OM.o( oid ), nullptr );

		Com.rollback();

		QVERIFY( OM.o( oid ) != 0 );

		OM.recycle( oid );

		OM.recycleObjects();
	}

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		lua_task::process( QString( "moo.create()" ), CID, Programmer->id() );

		O = OM.object( oid );

		QVERIFY( O != 0 );

		bool			 V = O->read();

		QString			 CMD = QString( "o( %1 ).r = %2" ).arg( oid ).arg( !V );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		QCOMPARE( O->read(), !V );

		Com.rollback();

		QCOMPARE( O->read(), V );

		OM.recycle( oid );

		OM.recycleObjects();
	}
}

void ServerTest::taskRollbackObjectProps( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*Owner      = OM.newObject();

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		lua_task::process( QString( "moo.create()" ), CID, Programmer->id() );

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QString			 CMD = QString( "o( %1 ):propadd( 'hello', 'world' );" ).arg( oid );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		QVERIFY( O->prop( "hello" )->value().toString() == "world" );

		Com.rollback();

		QCOMPARE( O->prop( "hello" ), nullptr);

		OM.recycle( oid );

		OM.recycleObjects();
	}
}

void ServerTest::taskRollbackObjectAliases( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*Owner      = OM.newObject();

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		lua_task::process( QString( "moo.create()" ), CID, Programmer->id() );

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QString			 CMD = QString( "o( %1 ):aliasadd( 'test' )" ).arg( oid );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		QVERIFY( O->aliases().contains( "test" ) );

		Com.rollback();

		QVERIFY( !O->aliases().contains( "test" ) );

		OM.recycle( oid );

		OM.recycleObjects();
	}

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		lua_task::process( QString( "moo.create()" ), CID, Programmer->id() );

		O = OM.object( oid );

		QVERIFY( O != 0 );

		lua_task::process( QString( "o( %1 ):aliasadd( 'test' )" ).arg( oid ), CID, Programmer->id() );

		QVERIFY( O->aliases().contains( "test" ) );

		QString			 CMD = QString( "o( %1 ):aliasdel( 'test' )" ).arg( oid );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		QVERIFY( !O->aliases().contains( "test" ) );

		Com.rollback();

		QVERIFY( O->aliases().contains( "test" ) );

		OM.recycle( oid );

		OM.recycleObjects();
	}
}
