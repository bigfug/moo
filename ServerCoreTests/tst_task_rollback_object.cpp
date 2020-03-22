#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void ServerTest::taskRollbackObjectCreate( void )
{
	LuaTestData			TD;

	ObjectId		 oid = TD.OM.maxId();
	Object			*O   = 0;

	lua_task		 Task = TD.eval( "moo.create()" );

	O = TD.OM.object( oid );

	QVERIFY( O != 0 );

	Task.rollback();

	QVERIFY( O->recycle() );
	QCOMPARE( TD.OM.o( O->id() ), nullptr );
}

void ServerTest::taskRollbackObjectRecycle()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	lua_task		 Com = TD.eval( QString( "o( %1 ):recycle()" ).arg( O->id() ) );

	QVERIFY( O->recycle() );

	QCOMPARE( TD.OM.o( O->id() ), nullptr );
	QCOMPARE( TD.OM.objectIncludingRecycled( O->id() ), O );

	Com.rollback();

	QCOMPARE( TD.OM.o( O->id() ), O );
}

void ServerTest::taskRollbackObjectRead()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->read();

	lua_task	 C = TD.eval( QString( "o( %1 ).r = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->read(), !V );

	C.rollback();

	QCOMPARE( O->read(), V );
}

void ServerTest::taskRollbackObjectWrite( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->write();

	lua_task	 C = TD.eval( QString( "o( %1 ).w = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->write(), !V );

	C.rollback();

	QCOMPARE( O->write(), V );
}

void ServerTest::taskRollbackObjectFertile( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->fertile();

	lua_task	 C = TD.eval( QString( "o( %1 ).f = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->fertile(), !V );

	C.rollback();

	QCOMPARE( O->fertile(), V );
}

void ServerTest::taskRollbackObjectName( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );

	O->setName( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).name = '%2'" ).arg( O->id() ).arg( v2 ) );

	QCOMPARE( O->name(), v2 );

	C.rollback();

	QCOMPARE( O->name(), v1 );
}

void ServerTest::taskRollbackObjectOwner()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	ObjectId		v1( 1 );
	ObjectId		v2( 2 );

	O->setOwner( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).owner = o( %2 )" ).arg( O->id() ).arg( v2 ) );

	QCOMPARE( O->owner(), v2 );

	C.rollback();

	QCOMPARE( O->owner(), v1 );
}

void ServerTest::taskRollbackObjectParent()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	ObjectId		v1( 1 );
	ObjectId		v2( 2 );

	O->setParent( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).parent = o( %2 )" ).arg( O->id() ).arg( v2 ) );

	QCOMPARE( O->parent(), v2 );

	C.rollback();

	QCOMPARE( O->parent(), v1 );
}

void ServerTest::taskRollbackObjectLocation()
{
	LuaTestData			TD;

	Object		*O  = TD.OM.newObject();
	Object		*L1 = TD.OM.newObject();
	Object		*L2 = TD.OM.newObject();

	QVERIFY( O );

	O->move( L1 );

	QCOMPARE( O->location(), L1->id() );

	lua_task	 C = TD.eval( QString( "o( %1 ).location = o( %2 )" ).arg( O->id() ).arg( L2->id() ) );

	QCOMPARE( O->location(), L2->id() );

	C.rollback();

	QCOMPARE( O->location(), L1->id() );
}

void ServerTest::taskRollbackObjectPlayer()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->player();

	lua_task	 C = TD.eval( QString( "o( %1 ).player = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->player(), !V );

	C.rollback();

	QCOMPARE( O->player(), V );
}

void ServerTest::taskRollbackObjectProgrammer()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->programmer();

	lua_task	 C = TD.eval( QString( "o( %1 ).programmer = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->programmer(), !V );

	C.rollback();

	QCOMPARE( O->programmer(), V );
}

void ServerTest::taskRollbackObjectWizard()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->wizard();

	lua_task	 C = TD.eval( QString( "o( %1 ).wizard = %2" ).arg( O->id() ).arg( !V ) );

	QCOMPARE( O->wizard(), !V );

	C.rollback();

	QCOMPARE( O->wizard(), V );
}

void ServerTest::taskRollbackObjectPropAdd( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );

	lua_task	 C = TD.eval( QString( "o( %1 ):propadd( '%2', '%3' );" ).arg( O->id() ).arg( v1 ).arg( v2 ) );

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );

	C.rollback();

	QVERIFY( !O->prop( v1 ) );
}

void ServerTest::taskRollbackObjectAliasAdd( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );

	lua_task	 C = TD.eval( QString( "o( %1 ):aliasadd( '%2' )" ).arg( O->id() ).arg( v1 ) );

	QVERIFY( O->aliases().contains( v1 ) );

	C.rollback();

	QVERIFY( !O->aliases().contains( v1 ) );
}

void ServerTest::taskRollbackObjectAliasDelete()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );

	TD.process( QString( "o( %1 ):aliasadd( '%2' )" ).arg( O->id() ).arg( v1 ) );

	QVERIFY( O->aliases().contains( v1 ) );

	lua_task	 C = TD.eval( QString( "o( %1 ):aliasdel( '%2' )" ).arg( O->id() ).arg( v1 ) );

	QVERIFY( !O->aliases().contains( v1 ) );

	C.rollback();

	QVERIFY( O->aliases().contains( v1 ) );
}
