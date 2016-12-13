#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_prop.h"
#include "lua_verb.h"
#include "lua_task.h"

#include "luatestdata.h"

void ServerTest::luaVerbAdd( void )
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
		QString			 CMD = QString( "o( %1 ):verbadd( 'test' )" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test", OBJECT_NONE, "", OBJECT_NONE );

		QVERIFY( V != 0 );

		QCOMPARE( V->owner(), Programmer->id() );
		QCOMPARE( V->read(), true );
		QCOMPARE( V->write(), false );
		QCOMPARE( V->execute(), true );
		QCOMPARE( V->directObject(), ANY );
		QCOMPARE( V->indirectObject(), ANY );
		QCOMPARE( V->prepositionType(), ANY );
	}

	if( true )
	{
		QString			 CMD = QString( "v=o( %1 ):verb( 'test' );v.dobj = 'any';v.prep = 'from';v.iobj='this';" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test", OBJECT_NONE, "from", O->id() );

		QVERIFY( V != 0 );

		QCOMPARE( V->directObject(), ANY );
		QCOMPARE( V->prepositionType(), THIS );
		QCOMPARE( V->preposition(), QString( "from" ) );
		QCOMPARE( V->indirectObject(), THIS );
	}

	if( true )
	{
		QString			 CMD = QString( "v=o( %1 ):verb( 'test' );v.dobj = 'any';v.prep = 'none';v.iobj='any';" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test", OBJECT_NONE, "", OBJECT_NONE );

		QVERIFY( V != 0 );

		QCOMPARE( V->directObject(), ANY );
		QCOMPARE( V->prepositionType(), NONE );
		QCOMPARE( V->indirectObject(), ANY );
	}

	if( true )
	{
		QString			 CMD = QString( "o( %1 ):verb( 'test' ).script = 'moo.notify( \"test call\" );'" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test", OBJECT_NONE, "", OBJECT_NONE );

		QVERIFY( V != 0 );

		QCOMPARE( V->script(), QString( "moo.notify( \"test call\" );" ) );
	}

	if( true )
	{
		QString			 CMD = QString( "o( %1 ):test()" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test", OBJECT_NONE, "", OBJECT_NONE );

		QVERIFY( V != 0 );
	}

	ObjectManager::reset();
}

void ServerTest::luaVerbDel()
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
		QString			 CMD = QString( "o( %1 ):verbadd( 'test' )" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test" );

		QVERIFY( V != 0 );
	}

	if( true )
	{
		QString			 CMD = QString( "o( %1 ):verbdel( 'test' );" ).arg( O->id() );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		Verb			*V = O->verbMatch( "test" );

		QVERIFY( V == 0 );
	}

	ObjectManager::reset();
}
