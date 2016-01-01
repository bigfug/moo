#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "connectionmanager.h"

#include "lua_moo.h"
#include "lua_task.h"

void ServerTest::luaMoveTestValidWhat( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );

	QVERIFY( Programmer != 0 );

	if( true )
	{
		QString			 CMD = QString( "o( 123 ).location = o( %1 )" ).arg( Programmer->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		//lua_moo::stackDump( Com.L() );

		QVERIFY( Programmer->children().isEmpty() );
	}

	ObjectManager::reset();
}

void ServerTest::luaMoveToRoot( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );

	QVERIFY( Programmer != 0 );

	Object			*Object = OM.newObject();

	QVERIFY( OM.object( OM.maxId() ) == 0 );

	Object->setOwner( Programmer->id() );
	Object->move( Programmer );

	QCOMPARE( Programmer->contents().size(), 1 );
	QCOMPARE( Object->location(), Programmer->id() );

	if( true )
	{
		QString			 CMD = QString( "o( %1 ).location = -1" ).arg( Object->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		//lua_moo::stackDump( Com.L() );

		QVERIFY( Programmer->children().isEmpty() );
		QCOMPARE( Object->location(), -1 );
	}

	ObjectManager::reset();
}

void ServerTest::luaMove( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Child      = OM.newObject();

	Child->setOwner( Programmer->id() );

	if( true )
	{
		QString			 CMD = QString( "o( %1 ).location = o( %2 )" ).arg( Child->id() ).arg( Programmer->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		//qDebug() << Com.command();

		Com.eval();
	}

	ObjectManager::reset();
}

