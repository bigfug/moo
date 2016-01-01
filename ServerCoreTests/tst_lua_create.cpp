
#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

void ServerTest::luaCreate( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*Owner      = OM.newObject();

	QCOMPARE( Con.player(), Programmer->id() );

	//qDebug() << "SYSTEM";

	if( true )
	{
		QString			 CMD = QString( "moo.create()" );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), OBJECT_NONE );
		QCOMPARE( O->owner(), OBJECT_NONE );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ) )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		try
		{
			Com.eval();
		}
		catch( ... )
		{

		}

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), OBJECT_NONE );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), -1 )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), O->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), o( %2 ) )" ).arg( Parent->id() ).arg( Owner->id() );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Owner->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	//qDebug() << "WITH WIZARD";

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

		QCOMPARE( O->parent(), OBJECT_NONE );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ) )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		try
		{
			Com.eval();
		}
		catch( ... )
		{

		}

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), -1 )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), O->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), o( %2 ) )" ).arg( Parent->id() ).arg( Owner->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Owner->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	// Now test without wizard status

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

		QCOMPARE( O->parent(), OBJECT_NONE );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	Parent->setOwner( OBJECT_NONE );
	Parent->setFertile( true );

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ) )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		try
		{
			Com.eval();
		}
		catch( ... )
		{

		}

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	Parent->setOwner( Programmer->id() );
	Parent->setFertile( false );

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ) )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		try
		{
			Com.eval();
		}
		catch( ... )
		{

		}

		O = OM.object( oid );

		QVERIFY( O != 0 );

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	Parent->setOwner( OBJECT_NONE );
	Parent->setFertile( false );

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ) )" ).arg( Parent->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		try
		{
			Com.eval();
		}
		catch( ... )
		{

		}

		O = OM.object( oid );

		QVERIFY( O == 0 );
	}

	Parent->setOwner( Programmer->id() );
	Parent->setFertile( true );

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

		QCOMPARE( O->parent(), Parent->id() );
		QCOMPARE( O->owner(), Programmer->id() );

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	if( true )
	{
		QString			 CMD = QString( "moo.create( o( %1 ), o( %2 ) )" ).arg( Parent->id() ).arg( Owner->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		//qDebug() << Com.command();

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O == 0 );
	}

	Property	P;

	P.initialise();
	P.setOwner( Programmer->id() );
	P.setValue( (qint32)1 );

	Programmer->propAdd( "ownership_quota", P );

	QVERIFY( Programmer->prop( "ownership_quota" ) != 0 );

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

		QCOMPARE( Programmer->prop( "ownership_quota" )->value().toInt(), 0 );

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

			QVERIFY( O == 0 );

			QCOMPARE( Programmer->prop( "ownership_quota" )->value().toInt(), 0 );
		}

		O->setParent( OBJECT_NONE );

		OM.recycle( O );

		OM.recycleObjects();
	}

	ObjectManager::reset();
}

