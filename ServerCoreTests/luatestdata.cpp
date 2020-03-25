#include "luatestdata.h"

#include <QDateTime>

#include "tst_servertest.h"

LuaTestData::LuaTestData( void ) :
	OM( *ObjectManager::instance() ),
	CM( *ConnectionManager::instance() )
{
	TimeStamp = QDateTime::currentMSecsSinceEpoch();

	CID = ServerTest::initLua( TimeStamp );

	Con = CM.connection( CID );

	Programmer = OM.object( Con->player() );
}

LuaTestData::~LuaTestData()
{
	delete ConnectionManager::instance();

	ObjectManager::reset();
}

lua_task LuaTestData::execute(const QString &pCmd)
{
	lua_task	t( CID, TaskEntry( pCmd, CID, programmerId() ) );

	t.execute( TimeStamp );

	return( t );
}

lua_task LuaTestData::task(const QString &pCmd)
{
	return( task( pCmd, programmerId() ) );
}

lua_task LuaTestData::task( const QString &pCmd, ObjectId pProgrammerId )
{
	return( lua_task( CID, TaskEntry( pCmd, pProgrammerId ) ) );
}

lua_task LuaTestData::eval(const QString &pCmd)
{
	return( eval( pCmd, programmerId() ) );
}

lua_task LuaTestData::eval(const QString &pCmd, ObjectId pProgrammerId)
{
	lua_task	t( CID, TaskEntry( pCmd, CID, pProgrammerId ) );

	t.eval();

	return( t );
}

void LuaTestData::process( const QString &pCmd )
{
	process( pCmd, programmerId() );
}

void LuaTestData::process( const QString &pCmd, ObjectId pProgrammerId )
{
	lua_task::process( pCmd, CID, pProgrammerId );
}
