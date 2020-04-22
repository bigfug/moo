
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

void ServerTest::luaRegistration( void )
{
	try
	{
		lua_State		*L  = lua_moo::luaNewState();

		QVERIFY( L );

		lua_close( L );
	}
	catch( ... )
	{

	}
}

QTEST_GUILESS_MAIN( ServerTest )

#include "tst_servertest.moc"
