#include "task_rollback.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void TaskRollback::taskRollbackObjectCreate( void )
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

void TaskRollback::taskRollbackObjectRecycle()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	lua_task		 Com = TD.eval( QString( "o( %1 ):recycle()" ).arg( O->id() ), OBJECT_SYSTEM );

	QVERIFY( O->recycle() );

	QCOMPARE( TD.OM.o( O->id() ), nullptr );
	QCOMPARE( TD.OM.objectIncludingRecycled( O->id() ), O );

	Com.rollback();

	QCOMPARE( TD.OM.o( O->id() ), O );
}

void TaskRollback::taskRollbackObjectRead()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->read();

	lua_task	 C = TD.eval( QString( "o( %1 ).r = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->read(), !V );

	C.rollback();

	QCOMPARE( O->read(), V );
}

void TaskRollback::taskRollbackObjectWrite( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->write();

	lua_task	 C = TD.eval( QString( "o( %1 ).w = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->write(), !V );

	C.rollback();

	QCOMPARE( O->write(), V );
}

void TaskRollback::taskRollbackObjectFertile( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->fertile();

	lua_task	 C = TD.eval( QString( "o( %1 ).f = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->fertile(), !V );

	C.rollback();

	QCOMPARE( O->fertile(), V );
}

void TaskRollback::taskRollbackObjectName( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );

	O->setName( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).name = '%2'" ).arg( O->id() ).arg( v2 ), OBJECT_SYSTEM );

	QCOMPARE( O->name(), v2 );

	C.rollback();

	QCOMPARE( O->name(), v1 );
}

void TaskRollback::taskRollbackObjectOwner()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	ObjectId		v1( 1 );
	ObjectId		v2( 2 );

	O->setOwner( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).owner = o( %2 )" ).arg( O->id() ).arg( v2 ), OBJECT_SYSTEM );

	QCOMPARE( O->owner(), v2 );

	C.rollback();

	QCOMPARE( O->owner(), v1 );
}

void TaskRollback::taskRollbackObjectParent()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	ObjectId		v1( 1 );
	ObjectId		v2( 2 );

	O->setParent( v1 );

	lua_task	 C = TD.eval( QString( "o( %1 ).parent = o( %2 )" ).arg( O->id() ).arg( v2 ), OBJECT_SYSTEM );

	QCOMPARE( O->parent(), v2 );

	C.rollback();

	QCOMPARE( O->parent(), v1 );
}

void TaskRollback::taskRollbackObjectLocation()
{
	LuaTestData			TD;

	Object		*O  = TD.OM.newObject();
	Object		*L1 = TD.OM.newObject();
	Object		*L2 = TD.OM.newObject();

	QVERIFY( O );

	O->move( L1 );

	QCOMPARE( O->location(), L1->id() );

	lua_task	 C = TD.eval( QString( "o( %1 ).location = o( %2 )" ).arg( O->id() ).arg( L2->id() ), OBJECT_SYSTEM );

	QCOMPARE( O->location(), L2->id() );

	C.rollback();

	QCOMPARE( O->location(), L1->id() );
}

void TaskRollback::taskRollbackObjectPlayer()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->player();

	lua_task	 C = TD.eval( QString( "o( %1 ).player = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->player(), !V );

	C.rollback();

	QCOMPARE( O->player(), V );
}

void TaskRollback::taskRollbackObjectProgrammer()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->programmer();

	lua_task	 C = TD.eval( QString( "o( %1 ).programmer = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->programmer(), !V );

	C.rollback();

	QCOMPARE( O->programmer(), V );
}

void TaskRollback::taskRollbackObjectWizard()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	bool		 V = O->wizard();

	lua_task	 C = TD.eval( QString( "o( %1 ).wizard = %2" ).arg( O->id() ).arg( !V ), OBJECT_SYSTEM );

	QCOMPARE( O->wizard(), !V );

	C.rollback();

	QCOMPARE( O->wizard(), V );
}

void TaskRollback::taskRollbackObjectPropAdd( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );

	lua_task	 C = TD.eval( QString( "o( %1 ):propadd( '%2', '%3' );" ).arg( O->id() ).arg( v1 ).arg( v2 ), OBJECT_SYSTEM );

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );

	C.rollback();

	QVERIFY( !O->prop( v1 ) );
}

void TaskRollback::taskRollbackObjectPropDelete()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );

	TD.process( QString( "o( %1 ):propadd( '%2', '%3' );" ).arg( O->id() ).arg( v1 ).arg( v2 ), OBJECT_SYSTEM );

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );

	lua_task	 C = TD.eval( QString( "o( %1 ):propdel( '%2' );" ).arg( O->id() ).arg( v1 ), OBJECT_SYSTEM );

	QVERIFY( !O->prop( v1 ) );

	C.rollback();

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );
}

void TaskRollback::taskRollbackObjectPropClear()
{
	LuaTestData			TD;

	Object		*O1 = TD.OM.newObject();
	Object		*O2 = TD.OM.newObject();

	QVERIFY( O1 );
	QVERIFY( O2 );

	O2->setParent( O1->id() );

	QString		v1( "test1" );
	QString		v2( "test2" );
	QString		v3( "test3" );

	QVERIFY( v2 != v3 );

	TD.process( QString( "o( %1 ):propadd( '%2', '%3' );" ).arg( O1->id() ).arg( v1 ).arg( v2 ), OBJECT_SYSTEM );

	QVERIFY( O1->prop( v1 ) );
	QVERIFY( !O2->prop( v1 ) );
	QVERIFY( O2->propParent( v1 ) );
	QCOMPARE( O2->propParent( v1 )->value().toString(), v2 );

	TD.process( QString( "o( %1 ).%2 = '%3'" ).arg( O2->id() ).arg( v1 ).arg( v3 ), OBJECT_SYSTEM );

	QVERIFY( O2->prop( v1 ) );
	QCOMPARE( O2->prop( v1 )->value().toString(), v3 );

	lua_task	 C = TD.eval( QString( "o( %1 ):propclr( '%2' );" ).arg( O2->id() ).arg( v1 ), OBJECT_SYSTEM );

	QVERIFY( !O2->prop( v1 ) );
	QVERIFY( O2->propParent( v1 ) );

	C.rollback();

	QVERIFY( O2->prop( v1 ) );
	QCOMPARE( O2->prop( v1 )->value().toString(), v3 );
}

void TaskRollback::taskRollbackObjectPropValue()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );
	QString		v2( "test2" );
	QString		v3( "test3" );

	QVERIFY( v2 != v3 );

	TD.process( QString( "o( %1 ):propadd( '%2', '%3' );" ).arg( O->id() ).arg( v1 ).arg( v2 ), OBJECT_SYSTEM );

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );

	lua_task	 C = TD.eval( QString( "o( %1 ).%2 = '%3';" ).arg( O->id() ).arg( v1 ).arg( v3 ), OBJECT_SYSTEM );

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v3 );

	C.rollback();

	QVERIFY( O->prop( v1 ) );

	QCOMPARE( O->prop( v1 )->value().toString(), v2 );
}

void TaskRollback::taskRollbackObjectAliasAdd( void )
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );

	lua_task	 C = TD.eval( QString( "o( %1 ):aliasadd( '%2' )" ).arg( O->id() ).arg( v1 ), OBJECT_SYSTEM );

	QVERIFY( O->aliases().contains( v1 ) );

	C.rollback();

	QVERIFY( !O->aliases().contains( v1 ) );
}

void TaskRollback::taskRollbackObjectAliasDelete()
{
	LuaTestData			TD;

	Object		*O = TD.OM.newObject();

	QVERIFY( O );

	QString		v1( "test1" );

	TD.process( QString( "o( %1 ):aliasadd( '%2' )" ).arg( O->id() ).arg( v1 ), OBJECT_SYSTEM );

	QVERIFY( O->aliases().contains( v1 ) );

	lua_task	 C = TD.eval( QString( "o( %1 ):aliasdel( '%2' )" ).arg( O->id() ).arg( v1 ), OBJECT_SYSTEM );

	QVERIFY( !O->aliases().contains( v1 ) );

	C.rollback();

	QVERIFY( O->aliases().contains( v1 ) );
}

void TaskRollback::taskRollbackObjectPropInherit( void )
{
	LuaTestData			TD;

	Object		*O1 = TD.OM.newObject();
	Object		*O2 = TD.OM.newObject();

	TD.process( QString( "o( %1 ).parent = o( %2 )" ).arg( O2->id() ).arg( O1->id() ), OBJECT_SYSTEM );

	QCOMPARE( O2->parent(), O1->id() );

	TD.process( QString( "o( %1 ):propadd( 'test', 'value1' )" ).arg( O1->id() ), OBJECT_SYSTEM );
	TD.process( QString( "o( %1 ).test = 'value2'" ).arg( O2->id() ), OBJECT_SYSTEM );

	QCOMPARE( O2->prop( "test" )->value().toString(), QStringLiteral( "value2" ) );
	QCOMPARE( O2->propParent( "test" )->value().toString(), QStringLiteral( "value1" ) );

	lua_task	 C = TD.eval( QString( "o( %1 ).parent = O_NONE" ).arg( O2->id() ), OBJECT_SYSTEM );

	QCOMPARE( O2->parent(), OBJECT_NONE );
	QCOMPARE( O2->prop( "test" ), nullptr );

	C.rollback();

	QCOMPARE( O2->parent(), O1->id() );
	QVERIFY( O2->prop( "test" ) );
	QVERIFY( O2->propParent( "test" ) );
	QCOMPARE( O2->prop( "test" )->value().toString(), QStringLiteral( "value2" ) );
	QCOMPARE( O2->propParent( "test" )->value().toString(), QStringLiteral( "value1" ) );
}

QTEST_GUILESS_MAIN( TaskRollback )

#include "task_rollback.moc"
