
#include "tst_servertest.h"
#include "permissionstest.h"

#include <iostream>

#include "objectmanager.h"
#include "connection.h"
#include "object.h"

#include "lua_moo.h"
#include "lua_object.h"
#include "lua_verb.h"
#include "lua_connection.h"
#include "lua_task.h"
#include "lua_prop.h"
#include "connectionmanager.h"

ServerTest::ServerTest()
{
}

ConnectionId ServerTest::initLua( qint64 pTimeStamp )
{
	if( ObjectManager::instance()->maxId() == 0 )
	{
		ObjectManager::instance()->luaMinimal();
	}

	ConnectionId	CID = ConnectionManager::instance()->doConnect( 0 );

	Task			 Tsk;
	lua_task		 Com( CID, Tsk );

	Com.execute( pTimeStamp );

	return( CID );
}

void ServerTest::initTestCase( void )
{
	lua_moo::initialiseAll();
}

void ServerTest::cleanupTestCase()
{
}

void ServerTest::luaRegistration( void )
{
	try
	{
		lua_State		*L  = luaL_newstate();

		QVERIFY( L );

		lua_moo::luaNewState( L );

		lua_close( L );
	}
	catch( ... )
	{

	}
}

QTEST_GUILESS_MAIN( ServerTest )

#include "tst_servertest.moc"
