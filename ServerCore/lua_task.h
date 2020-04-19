#ifndef LUA_TASK_H
#define LUA_TASK_H

#include "mooglobal.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <QList>
#include "task.h"
#include "changeset/changeset.h"

class Connection;
class Verb;

namespace change {
class Change;
}

class lua_task
{
public:
	// gets the currently active task for the given lua_State
	static lua_task *luaGetTask( lua_State *L );

	// sets the currently active task for the given lua_State
	static void luaSetTask( lua_State *L, lua_task *T );

public:
	lua_task( ConnectionId pConnectionId, const Task &pTask );

	lua_task( lua_task &&t );

	virtual ~lua_task( void );

	inline const Task &task( void ) const
	{
		return( mTasks.first() );
	}

	inline lua_State *L( void ) const
	{
		return( mL );
	}

	lua_State *L( void );

	Connection *connection( void ) const;

	int execute( qint64 pTimeStamp );

	int eval( void );

	void taskPush( const Task &T );
	void taskPop( void );

	int verbCall( ObjectId pObjectId, Verb *V, int pArgCnt = 0 );
	int verbCall( Task &pTask, Verb *V, int pArgCnt );

	int throwError( mooError pError, const QString pMessage = "" );

	inline ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

	inline qint64 timestamp( void ) const
	{
		return( mTimeStamp );
	}

	inline ObjectId permissions( void ) const
	{
		return( mTasks.first().permissions() );
	}

	inline void setPermissions( ObjectId pObjectId )
	{
		mTasks.first().setPermissions( pObjectId );
	}

	QStringList taskVerbStack( void ) const;

	inline void changeAdd( change::Change *pChange )
	{
		mChanges.add( pChange );
	}

	static int process( QString pCommand, ConnectionId pConnectionId = CONNECTION_NONE, ObjectId pPlayerId = OBJECT_NONE );

private:
	int subeval( void );

	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaTask( lua_State *L );
	static int luaKill( lua_State *L );
	static int luaSchedule( lua_State *L );

	static int luaObject( lua_State *L );
	static int luaVerb( lua_State *L );
	static int luaArgs( lua_State *L );
	static int luaArgStr( lua_State *L );
	static int luaCaller( lua_State *L );
	static int luaPlayer( lua_State *L );
	static int luaHere( lua_State *L );
	static int luaMe( lua_State *L );
	static int luaDirectObject( lua_State *L );
	static int luaDirectObjectString( lua_State *L );
	static int luaIndirectObject( lua_State *L );
	static int luaIndirectObjectString( lua_State *L );
	static int luaPreposition( lua_State *L );

	static int luaPermissions( lua_State *L );
	static int luaSetPermissions( lua_State *L );

	static void luaHook( lua_State *L, lua_Debug *ar );

	int execute( void );
	int executeLogin( void );

	int verbCallCode( Verb *V, int pArgCnt = 0 );

	static void *luaAlloc( void *ud, void *ptr, size_t osize, size_t nsize );
	void *luaAlloc( void *ptr, size_t osize, size_t nsize );

	void commit( void )
	{
		mChanges.commit();
	}

	void rollback( void )
	{
		mChanges.rollback();
	}

private:
	lua_State					*mL;
	ConnectionId				 mConnectionId;
	QList<Task>					 mTasks;
	qint64						 mTimeStamp; // when the current task was started
	size_t						 mMemUse;
	change::ChangeSet			 mChanges;
	bool						 mError;

	static const luaL_Reg		 mLuaStatic[];
	static const luaL_Reg		 mLuaGet[];
	static const luaL_Reg		 mLuaSet[];

	friend class lua_moo;
	friend class ServerTest;

	Q_DISABLE_COPY( lua_task )
};

#endif // LUA_TASK_H
