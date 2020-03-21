
#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_task.h"

void ServerTest::luaParentTestValidObject( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );

	if( true )
	{
		lua_task::process( QString( "o( 123 ).parent = o( %1 )" ).arg( Programmer->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		QVERIFY( Programmer->children().isEmpty() );
	}

	ObjectManager::reset();
}

void ServerTest::luaParentTestValidParent( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*objObject  = OM.newObject();

	if( true )
	{
		lua_task::process( QString( "o( %1 ).parent = 123" ).arg( objObject->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		//QVERIFY( Programmer->children().isEmpty() );
	}

	ObjectManager::reset();
}

void ServerTest::luaParentBasic( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*objObject  = OM.newObject();

	objObject->setOwner( Programmer->id() );

	QCOMPARE( Parent->children().size(), 0 );
	QCOMPARE( objObject->parent(), OBJECT_NONE );

	if( true )
	{
		lua_task::process( QString( "o( %1 ).parent = %2" ).arg( objObject->id() ).arg( Parent->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		QCOMPARE( Parent->children().size(), 1 );
		QCOMPARE( objObject->parent(), Parent->id() );
	}

	objObject->setParent( OBJECT_NONE );

	ObjectManager::reset();
}

void ServerTest::luaParentBasicReparent( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent1   = OM.newObject();
	Object			*Parent2   = OM.newObject();
	Object			*objObject = OM.newObject();

	objObject->setOwner( Programmer->id() );
	objObject->setParent( Parent1->id() );

	QCOMPARE( Parent1->children().size(), 1 );
	QCOMPARE( objObject->parent(), Parent1->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ).parent = %2" ).arg( objObject->id() ).arg( Parent2->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		QCOMPARE( Parent1->children().size(), 0 );
		QCOMPARE( Parent2->children().size(), 1 );
		QCOMPARE( objObject->parent(), Parent2->id() );
	}

	objObject->setParent( OBJECT_NONE );

	ObjectManager::reset();
}

void ServerTest::luaParentLoopTest( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object				*Programmer = OM.object( Con.player() );
	Object				*O[ 4 ];

	for( int i = 0 ; i < 4 ; i++ )
	{
		O[ i ] = OM.newObject();

		O[ i ]->setOwner( Programmer->id() );

		if( i > 0 )
		{
			O[ i ]->setParent( O[ i - 1 ]->id() );
		}
	}

	QCOMPARE( O[ 0 ]->children().size(), 1 );
	QCOMPARE( O[ 1 ]->children().size(), 1 );
	QCOMPARE( O[ 2 ]->children().size(), 1 );
	QCOMPARE( O[ 3 ]->children().size(), 0 );

	QCOMPARE( O[ 0 ]->parent(), OBJECT_NONE );
	QCOMPARE( O[ 1 ]->parent(), O[ 0 ]->id() );
	QCOMPARE( O[ 2 ]->parent(), O[ 1 ]->id() );
	QCOMPARE( O[ 3 ]->parent(), O[ 2 ]->id() );

	if( true )
	{
		lua_task::process( QString( "o( %1 ).parent = %2" ).arg( O[ 2 ]->id() ).arg( O[ 0 ]->id() ), CID, Programmer->id() );

		QCOMPARE( O[ 0 ]->children().size(), 1 );
		QCOMPARE( O[ 1 ]->children().size(), 1 );
		QCOMPARE( O[ 2 ]->children().size(), 1 );
		QCOMPARE( O[ 3 ]->children().size(), 0 );

		QCOMPARE( O[ 0 ]->parent(), OBJECT_NONE );
		QCOMPARE( O[ 1 ]->parent(), O[ 0 ]->id() );
		QCOMPARE( O[ 2 ]->parent(), O[ 1 ]->id() );
		QCOMPARE( O[ 3 ]->parent(), O[ 2 ]->id() );
	}

	for( int i = 3 ; i >= 0 ; i-- )
	{
		O[ i ]->setParent( OBJECT_NONE );

		OM.recycle( O[ i ] );
	}

	ObjectManager::reset();
}

void ServerTest::luaParentTestIsParentOf( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object				*Programmer = OM.object( Con.player() );
	Object				*O[ 3 ];

	for( int i = 0 ; i < 3 ; i++ )
	{
		O[ i ] = OM.newObject();

		O[ i ]->setOwner( Programmer->id() );

		if( i > 0 )
		{
			O[ i ]->setParent( O[ i - 1 ]->id() );
		}
	}

	QCOMPARE( O[ 0 ]->children().size(), 1 );
	QCOMPARE( O[ 1 ]->children().size(), 1 );
	QCOMPARE( O[ 2 ]->children().size(), 0 );

	QCOMPARE( O[ 0 ]->parent(), OBJECT_NONE );
	QCOMPARE( O[ 1 ]->parent(), O[ 0 ]->id() );
	QCOMPARE( O[ 2 ]->parent(), O[ 1 ]->id() );

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isParentOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	for( int i = 2 ; i >= 0 ; i-- )
	{
		O[ i ]->setParent( OBJECT_NONE );

		OM.recycle( O[ i ] );
	}

	ObjectManager::reset();
}

void ServerTest::luaParentTestIsChildOf( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object				*Programmer = OM.object( Con.player() );
	Object				*O[ 3 ];

	for( int i = 0 ; i < 3 ; i++ )
	{
		O[ i ] = OM.newObject();

		O[ i ]->setOwner( Programmer->id() );

		if( i > 0 )
		{
			O[ i ]->setParent( O[ i - 1 ]->id() );
		}
	}

	QCOMPARE( O[ 0 ]->children().size(), 1 );
	QCOMPARE( O[ 1 ]->children().size(), 1 );
	QCOMPARE( O[ 2 ]->children().size(), 0 );

	QCOMPARE( O[ 0 ]->parent(), OBJECT_NONE );
	QCOMPARE( O[ 1 ]->parent(), O[ 0 ]->id() );
	QCOMPARE( O[ 2 ]->parent(), O[ 1 ]->id() );

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 0 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 1 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 0 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 1 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, true );
	}

	if( true )
	{
		QString			 CMD = QString( "return( o( %1 ):isChildOf( o( %2 ) ) )" ).arg( O[ 2 ]->id() ).arg( O[ 2 ]->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		lua_task::luaSetTask( Com.L(), &Com );

		int				 R = Com.eval();

		QCOMPARE( R, 1 );

		bool			B = lua_toboolean( Com.L(), -1 );

		QCOMPARE( B, false );
	}

	for( int i = 2 ; i >= 0 ; i-- )
	{
		O[ i ]->setParent( OBJECT_NONE );

		OM.recycle( O[ i ] );
	}

	ObjectManager::reset();
}
