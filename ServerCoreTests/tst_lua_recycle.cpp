
#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_task.h"

void ServerTest::luaRecycle( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	//Object			*Owner      = OM.newObject();
	Object			*Child      = OM.newObject();

	QCOMPARE( Con.player(), Programmer->id() );

	Child->setOwner( Programmer->id() );

	//qDebug() << "WITH WIZARD";

	if( true )
	{
		QString			 CMD = QString( "moo.create( -1, %1 )" ).arg( Programmer->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->owner(), Programmer->id() );

		if( true )
		{
			QString			 CMD = QString( "o( %1 ):recycle()" ).arg( O->id() );
			TaskEntry		 TE( CMD, CID, Programmer->id() );
			lua_task		 Com( CID, TE );

			Com.eval();

			QCOMPARE( O->recycle(), true );
		}

		OM.recycleObjects();

		//QVERIFY( OM.objectCount() == 4 );
	}

	//qDebug() << "WITHOUT WIZARD";

	Programmer->setWizard( false );

	if( true )
	{
		QString			 CMD = QString( "moo.create()" );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		O->setOwner( OBJECT_NONE );

		if( true )
		{
			QString			 CMD = QString( "o( %1 ):recycle()" ).arg( O->id() );
			TaskEntry		 TE( CMD, CID, Programmer->id() );
			lua_task		 Com( CID, TE );

			Com.eval();

			QCOMPARE( O->recycle(), false );
		}

		OM.recycle( O );

		OM.recycleObjects();

		//QVERIFY( OM.objectCount() == 4 );
	}

	Parent->setFertile( true );
	Parent->setOwner( Programmer->id() );

	//qDebug() << "asdoaisjdoaijdoasijd";

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), o( %2 ) )" ).arg( Parent->id() ).arg( Programmer->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		O->setParent( Parent->id() );

		{
			QList<ObjectId>		idList;

			Parent->descendants( idList );

			QCOMPARE( idList.size(), 1 );
		}

		{
			QList<ObjectId>		idList;

			O->ancestors( idList );

			QCOMPARE( idList.size(), 1 );
		}

		QCOMPARE( O->owner(), Programmer->id() );

		QCOMPARE( Child->owner(), Programmer->id() );

		Child->setParent( O->id() );

		QCOMPARE( O->children().size(), 1 );

		//qDebug() << "o:recycle()";

		if( true )
		{
			QString			 CMD = QString( "o( %1 ):recycle()" ).arg( O->id() );
			TaskEntry		 TE( CMD, CID, Programmer->id() );
			lua_task		 Com( CID, TE );

			Com.eval();

			QCOMPARE( O->children().size(), 0 );
			QCOMPARE( Parent->children().size(), 1 );
			QCOMPARE( Child->parent(), Parent->id() );
			QCOMPARE( O->recycle(), true );
		}

		Child->setParent( OBJECT_NONE );

		OM.recycleObjects();

		//QVERIFY( OM.objectCount() == 4 );
	}

	ObjectManager::reset();
}
