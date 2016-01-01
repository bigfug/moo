#include "luatestdata.h"

#include <QDateTime>

#include "tst_servertest.h"

LuaTestData::LuaTestData( void ) :
	OM( *ObjectManager::instance() ),
	CM( *ConnectionManager::instance() ),
	Com( 0 )
{
	TimeStamp = QDateTime::currentMSecsSinceEpoch();

	CID = ServerTest::initLua( TimeStamp );

	Con = CM.connection( CID );

	Programmer = OM.object( Con->player() );
}

LuaTestData::~LuaTestData()
{
	if( Com != 0 )
	{
		delete Com;
	}

	ObjectManager::reset();
}

void LuaTestData::initTask(const QString &pCmd, ObjectId pProgrammerId)
{
	CMD = QString( pCmd );
	TE = TaskEntry( CMD, CID, pProgrammerId );
	Com = new lua_task( CID, TE );
}
