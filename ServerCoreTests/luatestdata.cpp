#include "luatestdata.h"

#include <QDateTime>

#include "lua_moo.h"

LuaTestData::LuaTestData( void ) :
	OM( *ObjectManager::instance() ),
	CM( *ConnectionManager::instance() )
{
	TimeStamp = QDateTime::currentMSecsSinceEpoch();

	CID = LuaTestObject::initLua( TimeStamp );

	Con = CM.connection( CID );

	Q_ASSERT( Con );

	Programmer = OM.object( Con->player() );

	Q_ASSERT( Programmer );
}

LuaTestData::~LuaTestData()
{
	ConnectionManager::reset();

	ObjectManager::reset();
}

lua_task LuaTestData::execute( const QString &pCmd, bool pElevated )
{
	lua_task	t( CID, TaskEntry( pCmd, CID, programmerId() ), pElevated );

	t.execute( TimeStamp );

	return( t );
}

lua_task LuaTestData::task( const QString &pCmd, bool pElevated )
{
	return( task( pCmd, programmerId(), pElevated ) );
}

lua_task LuaTestData::task( const QString &pCmd, ObjectId pProgrammerId, bool pElevated )
{
	return( lua_task( CID, TaskEntry( pCmd, pProgrammerId ), pElevated ) );
}

lua_task LuaTestData::eval( const QString &pCmd, bool pElevated )
{
	return( eval( pCmd, programmerId(), pElevated ) );
}

lua_task LuaTestData::eval( const QString &pCmd, ObjectId pProgrammerId, bool pElevated )
{
	lua_task	t( CID, TaskEntry( pCmd, CID, pProgrammerId ) );

	t.setElevated( pElevated );

	t.eval();

	return( t );
}

void LuaTestData::process( const QString &pCmd, bool pElevated )
{
	process( pCmd, programmerId(), pElevated );
}

void LuaTestData::process( const QString &pCmd, ObjectId pProgrammerId, bool pElevated )
{
	lua_task::process( pCmd, CID, pProgrammerId, pElevated );
}

ConnectionId LuaTestObject::initLua( qint64 pTimeStamp )
{
	if( !ObjectManager::instance()->maxId() )
	{
		ObjectManager::instance()->luaMinimal();
	}

	ConnectionId	CID = ConnectionManager::instance()->doConnect( 0 );

	Task			 Tsk;
	lua_task		 Com( CID, Tsk );

	Com.execute( pTimeStamp );

	return( CID );
}

void LuaTestObject::initTestCase()
{
	lua_moo::initialiseAll();
}

void LuaTestObject::cleanupTestCase()
{
	ConnectionManager::reset();
}
