#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

void ServerTest::taskRollbackObject( void )
{
	ObjectManager		&OM = *ObjectManager::instance();
	ConnectionManager	&CM = *ConnectionManager::instance();
	qint64				 TimeStamp = QDateTime::currentMSecsSinceEpoch();
	ConnectionId		 CID = initLua( TimeStamp );
	Connection			&Con = *CM.connection( CID );

	Object			*Programmer = OM.object( Con.player() );
	Object			*Parent     = OM.newObject();
	Object			*Owner      = OM.newObject();

	if( true )
	{
		ObjectId		 oid = OM.maxId();
		Object			*O   = 0;

		QString			 CMD = QString( "moo.create()" );
		TaskEntry		 TE( CMD, CID, Programmer->id() );
		lua_task		 Com( CID, TE );

		Com.eval();

		O = OM.object( oid );

		QVERIFY( O != 0 );

		Com.rollback();

		QVERIFY( O->recycle() );

		OM.recycleObjects();
	}
}
