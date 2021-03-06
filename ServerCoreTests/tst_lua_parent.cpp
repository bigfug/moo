
#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_task.h"
#include "luatestdata.h"
#include "objectlogic.h"

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
		lua_task::process( QString( "o( %1 ).parent = o( %2 )" ).arg( objObject->id() ).arg( Parent->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		QCOMPARE( Parent->children().size(), 1 );
		QCOMPARE( objObject->parent(), Parent->id() );
	}

	objObject->setParent( OBJECT_NONE );

	ObjectManager::reset();
}

void ServerTest::luaParentBasicReparent( void )
{
	LuaTestData			 TD;

	Object			*Parent1   = TD.OM.newObject();
	Object			*Parent2   = TD.OM.newObject();
	Object			*objObject = TD.OM.newObject();

	objObject->setOwner( TD.programmerId() );
	objObject->setParent( Parent1->id() );

	QCOMPARE( Parent1->children().size(), 1 );
	QCOMPARE( objObject->parent(), Parent1->id() );

	if( true )
	{
		TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( objObject->id() ).arg( Parent2->id() ) );

		//lua_moo::stackDump( Com.L() );

		QCOMPARE( Parent1->children().size(), 0 );
		QCOMPARE( Parent2->children().size(), 1 );
		QCOMPARE( objObject->parent(), Parent2->id() );
	}

	objObject->setParent( OBJECT_NONE );
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

void ServerTest::luaParentChangePropTest1( void )
{
	LuaTestData			TD;

	Object		*P1 = TD.OM.newObject();
	Object		*P2 = TD.OM.newObject();
	Object		*TO = TD.OM.newObject();

	P1->propAdd( "test", QVariant::fromValue<double>( 0 ) );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( TO->id() ).arg( P1->id() ), OBJECT_SYSTEM );

	QVERIFY( !TO->prop( "test" ) );
	QVERIFY( TO->propParent( "test" ) );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( TO->id() ).arg( P2->id() ), OBJECT_SYSTEM );

	QVERIFY( !TO->propParent( "test" ) );
}

void ServerTest::luaParentChangePropTest2( void )
{
	LuaTestData			TD;

	Object		*P1 = TD.OM.newObject();
	Object		*TO = TD.OM.newObject();

	P1->propAdd( "test", QVariant::fromValue<double>( 0 ) );
	TO->propAdd( "test", QVariant::fromValue<QString>( "test" ) );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( TO->id() ).arg( P1->id() ) );

	QVERIFY( TO->prop( "test" ) );

	QCOMPARE( TO->parent(), OBJECT_NONE );
}

void ServerTest::luaParentChangePropTest3( void )
{
	LuaTestData			TD;

	Object		*P1 = TD.OM.newObject();
	Object		*C1 = TD.OM.newObject();
	Object		*TO = TD.OM.newObject();

	P1->propAdd( "test", QVariant::fromValue<double>( 0 ) );
	C1->propAdd( "test", QVariant::fromValue<double>( 5 ) );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( C1->id() ).arg( TO->id() ), OBJECT_SYSTEM );

	QCOMPARE( C1->parent(), TO->id() );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( TO->id() ).arg( P1->id() ), OBJECT_SYSTEM );

	QCOMPARE( TO->parent(), OBJECT_NONE );
}

void ServerTest::luaParentChangePropTest4( void )
{
	LuaTestData			TD;

	Object		*P1 = TD.OM.newObject();
	Object		*TO = TD.OM.newObject();

	P1->propAdd( "test", QVariant::fromValue<double>( 0 ) );

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( TO->id() ).arg( P1->id() ), OBJECT_SYSTEM );

	QCOMPARE( TO->parent(), P1->id() );

	TO->propSet( "test", QVariant::fromValue<double>( 5 ) );

	QVERIFY( TO->prop( "test" ) );

	QCOMPARE( TO->prop( "test" )->parent(), P1->id() );

	TD.process( QString( "o( %1 ).parent = O_NONE" ).arg( TO->id() ) );

	QCOMPARE( TO->parent(), OBJECT_NONE );

	QCOMPARE( TO->prop( "test" ), nullptr );
}
