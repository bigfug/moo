#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "connectionmanager.h"
#include "luatestdata.h"

#include "lua_moo.h"
#include "lua_task.h"

void ServerTest::luaMoveTestValidWhat( void )
{
	LuaTestData			 TD;

	TD.process( QString( "o( 123 ).location = o( %1 )" ).arg( TD.programmerId() ) );

	QVERIFY( TD.Programmer );

	QVERIFY( TD.Programmer->children().isEmpty() );
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
		lua_task::process( QString( "o( %1 ).location = O_NONE" ).arg( Object->id() ), CID, Programmer->id() );

		//lua_moo::stackDump( Com.L() );

		QVERIFY( Programmer->children().isEmpty() );
		QCOMPARE( Object->location(), OBJECT_NONE );
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
		lua_task::process( QString( "o( %1 ).location = o( %2 )" ).arg( Child->id() ).arg( Programmer->id() ), CID, Programmer->id() );
	}

	ObjectManager::reset();
}

