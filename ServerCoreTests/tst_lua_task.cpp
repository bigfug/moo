
#include "tst_servertest.h"

#include "objectmanager.h"
#include "connectionmanager.h"
#include "luatestdata.h"

#include "connection.h"
#include "object.h"
#include "lua_task.h"

void ServerTest::luaTaskTests( void )
{
	LuaTestData			 TD;

	TD.process( "moo.notify( \"PlayerId = \" .. moo.player.id )" );
}
