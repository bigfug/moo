
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
		QString			 CMD = QString( "moo.notify( \"PlayerId = \" .. moo.player.id )" );
		TaskEntry		 TE( CMD, CID );
		lua_task		 Com( CID, TE );

		Com.eval();
	}

	ObjectManager::reset();
}
