
#include "tst_servertest.h"

#include "objectmanager.h"
#include "connectionmanager.h"

#include "connection.h"
#include "object.h"
#include "lua_task.h"

void ServerTest::luaTaskTests( void )
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
		lua_task::process( "moo.notify( \"PlayerId = \" .. moo.player.id )", CID );
	}

	ObjectManager::reset();
}
