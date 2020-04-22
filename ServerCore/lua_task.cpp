#include <QDateTime>
#include <QDebug>
#include <iostream>

#include "lua_task.h"
#include "task.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_verb.h"
#include "lua_connection.h"
#include "verb.h"
#include "object.h"
#include "property.h"
#include "mooexception.h"
#include "connectionmanager.h"
#include "taskentry.h"
#include "inputsinkprogram.h"

#include "changeset/connectionnotify.h"

#include <iostream>

const luaL_Reg lua_task::mLuaStatic[] =
{
	{ "task", lua_task::luaTask },
	{ "kill", lua_task::luaKill },
	{ "schedule", lua_task::luaSchedule },
	{ 0, 0 }
};

const luaL_Reg lua_task::mLuaGet[] =
{
	{ "object", lua_task::luaObject },
	{ "verb", lua_task::luaVerb },
	{ "args", lua_task::luaArgs },
	{ "argstr", lua_task::luaArgStr },
	{ "caller", lua_task::luaCaller },
	{ "here", lua_task::luaHere },
	{ "me", lua_task::luaMe },
	{ "player", lua_task::luaPlayer },
	{ "dobj", lua_task::luaDirectObject },
	{ "dobjstr", lua_task::luaDirectObjectString },
	{ "iobj", lua_task::luaIndirectObject },
	{ "iobjstr", lua_task::luaIndirectObjectString },
	{ "prepstr", lua_task::luaPreposition },
	{ "permissions", lua_task::luaPermissions },
	{ 0, 0 }
};

const luaL_Reg lua_task::mLuaSet[] =
{
	{ "permissions", lua_task::luaSetPermissions },
	{ 0, 0 }
};

void lua_task::initialise()
{
	lua_moo::addFunctions( mLuaStatic );
	lua_moo::addGet( mLuaGet );
	lua_moo::addSet( mLuaSet );
}

void lua_task::luaRegisterState( lua_State *L )
{
	Q_UNUSED( L )
}

int lua_task::luaObject( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.object() );

	return( 1 );
}

int lua_task::luaVerb( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_pushstring( L, T.verb().toLatin1() );

	return( 1 );
}

int lua_task::luaArgs( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();
	const QStringList	&W = T.args();

	lua_newtable( L );

	for( int i = 0 ; i < W.size() ; i++ )
	{
		lua_pushinteger( L, i + 1 );
		lua_pushstring( L, W.at( i ).toLatin1() );
		lua_settable( L, -3 );
	}

	return( 1 );
}

int lua_task::luaArgStr(lua_State *L)
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_pushstring( L, T.argstr().toLatin1() );

	return( 1 );
}

int lua_task::luaCaller( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.caller() );

	return( 1 );
}

int lua_task::luaPlayer( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.player() );

	return( 1 );
}

int lua_task::luaHere( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();
	Object				*O = ObjectManager::o( T.player() );

	lua_object::lua_pushobjectid( L, O == 0 ? OBJECT_NONE : O->location() );

	return( 1 );
}

int lua_task::luaMe( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.player() );

	return( 1 );
}

int lua_task::luaTask( lua_State *L )
{
	luaL_checkinteger( L, 1 );
	luaL_checkstring( L, 2 );

	lua_task			*LT = lua_task::luaGetTask( L );

	TaskEntry			E( QString( lua_tostring( L, 2 ) ), LT->connectionId(), LT->permissions() );

	E.setTimeStamp( E.timestamp() + ( lua_tonumber( L, 1 ) * 1000.0 ) );

	ObjectManager::instance()->queueTask( E );

	lua_pushinteger( L, E.id() );

	return( 1 );
}

int lua_task::luaKill(lua_State *L)
{
	luaL_checkinteger( L, 1 );

	TaskId		tid = lua_tointeger( L, 1 );

	lua_pushboolean( L, ObjectManager::instance()->killTask( tid ) );

	return( 1 );
}

int lua_task::luaSchedule( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		TaskEntrySchedule	 TS;
		QString				 TaskCode;

		if( lua_gettop( L ) == 1 )
		{
			if( lua_istable( L, 1 ) )
			{
				TS.mMinute = TS.mHour = TS.mDayOfWeek = TS.mDayOfMonth = TS.mMonth = TS.mYear = "*";

				lua_pushvalue( L, 1 );

				lua_pushnil( L );

				while( lua_next( L, -2 ) )
				{
					lua_pushvalue( L, -2 );

					const char *key   = lua_tostring( L, -1 );
					const char *value = lua_tostring( L, -2 );

					if( !strcmp( key, "minute" ) )
					{
						TS.mMinute = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "hour" ) )
					{
						TS.mHour = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "day_of_week" ) )
					{
						TS.mDayOfWeek = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "day_of_month" ) )
					{
						TS.mDayOfMonth = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "month" ) )
					{
						TS.mMonth = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "year" ) )
					{
						TS.mYear = QString::fromLatin1( value );
					}
					else if( !strcmp( key, "task" ) )
					{
						TaskCode= QString::fromLatin1( value );
					}

					lua_pop( L, 2 );
				}

				lua_pop( L, 1 );
			}
		}
		else if( lua_gettop( L ) == 7 )
		{
			const char		*Minute     = luaL_checkstring( L, 1 );
			const char		*Hour       = luaL_checkstring( L, 2 );
			const char		*DayOfWeek  = luaL_checkstring( L, 3 );
			const char		*DayOfMonth = luaL_checkstring( L, 4 );
			const char		*Month      = luaL_checkstring( L, 5 );
			const char		*Year       = luaL_checkstring( L, 6 );
			const char		*Task       = luaL_checkstring( L, 7 );

			TS.mMinute     = QString::fromLatin1( Minute );
			TS.mHour       = QString::fromLatin1( Hour );
			TS.mDayOfWeek  = QString::fromLatin1( DayOfWeek );
			TS.mDayOfMonth = QString::fromLatin1( DayOfMonth );
			TS.mMonth      = QString::fromLatin1( Month );
			TS.mYear       = QString::fromLatin1( Year );

			TaskCode       = QString::fromLatin1( Task );
		}

		TaskEntry			E( TaskCode, Command->connectionId(), Command->permissions() );

		E.setSchedule( TS );

		E.updateTimestampFromSchedule( QDateTime::currentMSecsSinceEpoch() );

		ObjectManager::instance()->queueTask( E );

		lua_pushinteger( L, E.id() );

		return( 1 );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception( lua_gettop( L ) ) );
}

int lua_task::luaDirectObject( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.directObjectId() );

	return( 1 );
}

int lua_task::luaDirectObjectString(lua_State *L)
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_pushstring( L, T.directObjectName().toUtf8() );

	return( 1 );
}

int lua_task::luaIndirectObject( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_object::lua_pushobjectid( L, T.indirectObjectId() );

	return( 1 );
}

int lua_task::luaIndirectObjectString(lua_State *L)
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_pushstring( L, T.indirectObjectName().toUtf8() );

	return( 1 );
}

int lua_task::luaPreposition( lua_State *L )
{
	const Task			&T = lua_task::luaGetTask( L )->task();

	lua_pushstring( L, T.preposition().toUtf8() );

	return( 1 );
}

int lua_task::luaPermissions( lua_State *L )
{
	const lua_task			*LT = lua_task::luaGetTask( L );

	lua_object::lua_pushobjectid( L, LT->permissions() );

	return( 1 );
}

int lua_task::luaSetPermissions( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object				*O  = lua_object::argObj( L, -1 );

		if( !O )
		{
			throw mooException( E_ARGS, "invalid object" );
		}

		if( Command->permissions() != O->id() && !Command->isWizard() )
		{
			throw mooException( E_PERM, "can't set permissions" );
		}

		Command->setPermissions( O->id() );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

lua_task::lua_task(ConnectionId pConnectionId, const Task &pTask, bool pElevated )
	: mL( Q_NULLPTR ), mConnectionId( pConnectionId ), mTimeStamp( 0 ), mMemUse( 0 ),
	  mPermissions( OBJECT_NONE ), mElevated( pElevated )
{
	taskPush( pTask );
}

lua_task::lua_task( lua_task &&t )
	: mL( std::move( t.mL ) ),
	  mConnectionId( std::move( t.mConnectionId ) ),
	  mTasks( std::move( t.mTasks ) ),
	  mTimeStamp( std::move( t.mTimeStamp ) ),
	  mMemUse( std::move( t.mMemUse ) ),
	  mChanges( std::move( t.mChanges ) ),
	  mPermissions( std::move( t.mPermissions ) ),
	  mElevated( std::move( t.mElevated ) ),
	  mException( std::move( t.mException ) )
{
	lua_setallocf( mL, lua_task::luaAlloc, this );

	luaSetTask( mL, this );

	t.mL = nullptr;
	t.mConnectionId = CONNECTION_NONE;
	t.mMemUse = 0;
	t.mException = mooException();
	t.mTimeStamp = 0;
}

lua_task::~lua_task( void )
{
	while( !mTasks.isEmpty() )
	{
		taskPop();
	}

	if( mL )
	{
		lua_close( mL );

		mL = Q_NULLPTR;
	}

	if( mTimeStamp )
	{
		qint64		D = QDateTime::currentMSecsSinceEpoch() - mTimeStamp;

		ObjectManager::instance()->recordExecutionTime( D );
	}

	if( !error() )
	{
		mChanges.commit();
	}
	else
	{
		mChanges.rollback();
	}
}

lua_State *lua_task::L()
{
	if( !mL )
	{
		if( ( mL = lua_newstate( lua_task::luaAlloc, this ) ) == Q_NULLPTR )
		{
			return( Q_NULLPTR );
		}

		lua_moo::luaNewState( mL );

		lua_sethook( mL, &lua_task::luaHook, LUA_MASKCOUNT, 10 );

		luaSetTask( mL, this );

		mTimeStamp = QDateTime::currentMSecsSinceEpoch();
	}

	return( mL );
}

Connection *lua_task::connection() const
{
	return( ConnectionManager::instance()->connection( connectionId() ) );
}

static const char	 TaskLuaKey = 'k';

lua_task *lua_task::luaGetTask( lua_State *L )
{
	lua_pushlightuserdata( L, (void *)&TaskLuaKey );
	lua_gettable( L, LUA_REGISTRYINDEX );

	lua_task	*T = reinterpret_cast<lua_task *>( lua_touserdata( L, -1 ) );

	lua_pop( L, 1 );

	return( T );
}

void lua_task::luaSetTask( lua_State *L, lua_task *T )
{
	lua_pushlightuserdata( L, (void *)&TaskLuaKey );
	lua_pushlightuserdata( L, T );
	lua_settable( L, LUA_REGISTRYINDEX );
}

void *lua_task::luaAlloc( void *ptr, size_t osize, size_t nsize )
{
	mMemUse -= osize;

	if( nsize == 0 )
	{
		free(ptr);

		return Q_NULLPTR;
	}

	mMemUse += nsize;

	return realloc(ptr, nsize);
}

void *lua_task::luaAlloc( void *ud, void *ptr, size_t osize, size_t nsize )
{
	return( static_cast<lua_task *>( ud )->luaAlloc( ptr, osize, nsize ) );
}

int lua_task::execute( qint64 pTimeStamp )
{
	Connection		*C		= ConnectionManager::instance()->connection( connectionId() );
	Task			&T		= mTasks.front();

	if( C && C->processInput( T.command() ) )
	{
		return( 0 );
	}

	QString			ArgStr;
	QStringList		Words	= Verb::parse( T.command(), ArgStr );
	const QString	First	= ( Words.isEmpty() ? "" : Words.takeFirst() );

//	qDebug() << "First:" << First << "ArgStr:" << ArgStr << "Words:" << Words;

	mTimeStamp = pTimeStamp;

	// the server next checks to see if the first word names any of the six "built-in" commands:
	// `.program', `PREFIX', `OUTPUTPREFIX', `SUFFIX', `OUTPUTSUFFIX',
	// or the connection's defined flush command, if any (`.flush' by default).

	T.setCaller( T.player() );
	T.setVerb( First );
	T.setArgStr( ArgStr );
	T.setArgs( Words );

	taskDump( "execute()", T );

	// check for the built-in commands

	if( First == "PREFIX" || First == "OUTPUTPREFIX" )
	{
		if( C )
		{
			if( !Words.isEmpty() )
			{
				C->setPrefix( Words.at( 0 ) );
			}
			else
			{
				C->setPrefix( "" );
			}
		}

		return( 0 );
	}
	else if( First == "SUFFIX" || First == "OUTPUTSUFFIX" )
	{
		if( C )
		{
			if( !Words.isEmpty() )
			{
				C->setSuffix( Words.at( 0 ) );
			}
			else
			{
				C->setSuffix( "" );
			}
		}

		return( 0 );
	}
	else if( First == ".flush" )
	{
		return( 0 );
	}

	// We're processing a normal command from the user

	if( !L() )
	{
		return( 0 );
	}

	if( ObjectManager::instance()->maxId() == 0 )
	{
		ObjectManager::instance()->luaMinimal();
	}

	if( C && C->player() == OBJECT_NONE )
	{
		return( executeLogin() );
	}

	return( execute() );
}

int lua_task::eval( void )
{
	lua_State		*LS			= L();	// initialise mL

	Task			&T			= mTasks.front();
	const int		 OldTop		= lua_gettop( LS );
	int				 Error;

	if( ( Error = luaL_loadstring( LS, T.command().toLatin1() ) ) == LUA_OK )
	{
		lua_getglobal( LS, "moo_sandbox" );

		lua_setupvalue( LS, -2, 1 );

		if( ( Error = lua_pcall( LS, 0, LUA_MULTRET, 0 ) ) == LUA_OK )
		{

		}
	}

	if( Error )
	{
		QString		Err = QString( lua_tostring( LS, -1 ) );

		Connection	*CON = ConnectionManager::instance()->connection( connectionId() );

		if( CON )
		{
			CON->notify( Err.toHtmlEscaped() );
		}

		std::cerr << "eval: " << Err.toStdString() << std::endl;

		lua_pop( LS, 1 );

		mException = mooException( E_EXCEPTION, Err );
	}

	const int			 NewTop = lua_gettop( LS );

	return( NewTop - OldTop );
}

int lua_task::subeval()
{
	return( 0 );
}

int lua_task::login( Object *pPlayer )
{
	try
	{
		Task						&T	= mTasks.front();
		ConnectionManager			&CM = *ConnectionManager::instance();
		const ConnectionNodeMap		&NM = CM.connectionList();
		ObjectManager				&OM	= *ObjectManager::instance();
		Object						*System = OM.systemObject();
		bool						 UR = false;
		Verb						*V;
		ObjectId					 MaxId			= OM.maxId();

		for( ConnectionNodeMap::const_iterator it = NM.begin() ; it != NM.end() ; it++ )
		{
			Connection		*C = it.value();

			if( C->player() == pPlayer->id() )
			{
				if( C->object() )
				{
					if( ( V = System->verbMatch( "user_client_disconnected" ) ) != Q_NULLPTR )
					{
						lua_object::lua_pushobject( mL, pPlayer );

						verbCall( System->id(), V, 1 );
					}
				}
				else
				{
					UR = true;
				}

				C->setPlayerId( OBJECT_NONE );
			}
		}

		CM.logon( connectionId(), pPlayer->id() );

		pPlayer->setConnection( connectionId() );

		T.setPlayer( pPlayer->id() );

		if( OM.maxId() > MaxId )
		{
			if( ( V = System->verbMatch( "user_created" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, pPlayer );

				verbCall( System->id(), V, 1 );
			}
		}
		else if( UR )
		{
			if( ( V = System->verbMatch( "user_reconnected" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, pPlayer );

				verbCall( System->id(), V, 1 );
			}
		}
		else
		{
			if( ( V = System->verbMatch( "user_connected" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, pPlayer );

				verbCall( System->id(), V, 1 );
			}
		}
	}
	catch( const mooException &e )
	{
		setException( e );
	}
	catch( ... )
	{
	}

	return( lua_pushexception( lua_gettop( mL ) ) );
}

int lua_task::executeLogin( void )
{
	try
	{
		Task			&T				= mTasks.front();
		ObjectManager	&OM				= *ObjectManager::instance();
		Object			*System			= OM.systemObject();
		Verb			*LoginCommand	= ( System ? System->verbMatch( "do_login_command" ) : Q_NULLPTR );

		if( !LoginCommand )
		{
			return( 0 );
		}

		T.setPermissions( System->owner() );

		QStringList		ArgLst = T.args();
		QString			ArgVrb = T.verb();

		if( !ArgVrb.isEmpty() )
		{
			ArgLst.prepend( ArgVrb );

			T.setArgs( ArgLst );
		}

		T.setVerb( LoginCommand->name() );

		if( verbCall( System->id(), LoginCommand ) != 1 )
		{
			lua_pop( mL, std::max<int>( 0, lua_gettop( mL ) ) );

			Q_ASSERT( lua_gettop( mL ) == 0 );

			return( 0 );
		}

		Object	*Player = lua_object::argObj( mL, -1 );

		lua_pop( mL, 1 );

		Q_ASSERT( lua_gettop( mL ) == 0 );

		if( !Player )
		{
			return( 0 );
		}

		if( !Player->player() )
		{
			return( 0 );
		}

		return( login( Player ) );
	}
	catch( const mooException &e )
	{
		setException( e );
	}
	catch( ... )
	{
	}

	return( lua_pushexception( lua_gettop( mL ) ) );
}

int lua_task::execute( void )
{
	try
	{
		Task			&T = mTasks.front();
		Connection		*C = ConnectionManager::instance()->connection( connectionId() );
		ObjectManager	&OM = *ObjectManager::instance();

		// The server next gives code in the database a chance to handle the command.

		// If the verb $do_command() exists, it is called with the words of the command passed as its
		//   arguments and argstr set to the raw command typed by the user.

		// If $do_command() does not exist, or if that verb-call completes normally (i.e., without
		//   suspending or aborting) and returns a false value, then the built-in command parser is
		//   invoked to handle the command as described below.

		Object		*Root = OM.systemObject();
		Verb		*DoCommand = ( Root ? Root->verbMatch( "do_command" ) : 0 );

		if( DoCommand && verbCall( *Root, DoCommand ) == 1 && lua_toboolean( mL, -1 ) )
		{
			return( lua_gettop( mL ) );
		}

		// Otherwise, it is assumed that the database code handled the command completely and no further
		//   action is taken by the server for that command.

		// The first word is taken to be the verb

		// The server then tries to find one of the prepositional phrases using the match that occurs
		//   earliest in the command.

		int		PrpIdx = T.findPreposition( T.args() );

		// If the server succeeds in finding a preposition, it considers the words between
		// the verb and the preposition to be the direct object and those after the preposition
		// to be the indirect object.

		// If there was no preposition, then the direct object is taken to be all of the words after
		// the verb and the indirect object is the empty string.

		T.getDirectAndIndirect( T.args(), PrpIdx );

		// The next step is to try to find MOO objects that are named by the
		//   direct and indirect object strings.

		// First, if an object string is empty, then the corresponding object is the special object #-1
		//   (aka $nothing in LambdaCore)
		// If an object string has the form of an object number (i.e., a hash mark (`#') followed by digits),
		//   and the object with that number exists, then that is the named object.
		// If the object string is either "me" or "here", then the player object itself or its location
		//   is used, respectively.

		QList<ObjectId>	ObjectIdList;

		if( T.directObjectName().isEmpty() )
		{
			T.setDirectObjectId( OBJECT_NONE );
		}
		else
		{
			T.setDirectObjectId( OBJECT_FAILED_MATCH );

			T.findObject( T.directObjectName(), ObjectIdList );

			if( ObjectIdList.size() > 1 )
			{
				T.setDirectObjectId( OBJECT_AMBIGUOUS );
			}
			else if( !ObjectIdList.isEmpty() )
			{
				T.setDirectObjectId( ObjectIdList.at( 0 ) );
			}

			ObjectIdList.clear();
		}

		if( T.indirectObjectName().isEmpty() )
		{
			T.setIndirectObjectId( OBJECT_NONE );
		}
		else
		{
			T.setIndirectObjectId( OBJECT_FAILED_MATCH );

			T.findObject( T.indirectObjectName(), ObjectIdList );

			if( ObjectIdList.size() > 1 )
			{
				T.setDirectObjectId( OBJECT_AMBIGUOUS );
			}
			else if( !ObjectIdList.isEmpty() )
			{
				T.setIndirectObjectId( ObjectIdList.at( 0 ) );
			}

			ObjectIdList.clear();
		}

		// It then looks at each of the verbs defined on each of the following four objects, in order:
		// 1. the player who typed the command
		// 2. the room the player is in
		// 3. the direct object, if any
		// 4. the indirect object, if any

		Object			*Player   = OM.object( T.player() );
		Object			*Location = ( Player != 0 ? OM.object( Player->location() ) : 0 );
		ObjectId		 DirectObjectId = T.directObjectId();
		ObjectId		 IndirectObjectId = T.indirectObjectId();
		Object			*DirectObject   = OM.object( DirectObjectId );
		Object			*IndirectObject = OM.object( IndirectObjectId );
		Verb			*FndVrb = Q_NULLPTR;
		Object			*FndObj = Q_NULLPTR;;

		if( Player && Player->verbFind( T.verb(), &FndVrb, &FndObj, DirectObjectId, T.preposition(), IndirectObjectId ) )
		{
			T.setObject( Player->id() );
		}
		else if( Location && Location->verbFind( T.verb(), &FndVrb, &FndObj, DirectObjectId, T.preposition(), IndirectObjectId ) )
		{
			T.setObject( Location->id() );
		}
		else if( DirectObject && DirectObject->verbFind( T.verb(), &FndVrb, &FndObj, DirectObjectId, T.preposition(), IndirectObjectId ) )
		{
			T.setObject( DirectObject->id() );
		}
		else if( IndirectObject && IndirectObject->verbFind( T.verb(), &FndVrb, &FndObj, DirectObjectId, T.preposition(), IndirectObjectId ) )
		{
			T.setObject( IndirectObject->id() );
		}
		else if( Location && Location->verbFind( "huh", &FndVrb, &FndObj ) )
		{
			T.setObject( Location->id() );
		}
		else
		{
			if( C )
			{
				changeAdd( new change::ConnectionNotify( C, "I couldn't understand that." ) );
			}

			return( 0 );
		}

		/*
		player    an object, the player who typed the command
		this      an object, the object on which this verb was found
		caller    an object, the same as `player'
		verb      a string, the first word of the command
		argstr    a string, everything after the first word of the command
		args      a list of strings, the words in `argstr'
		dobjstr   a string, the direct object string found during parsing
		dobj      an object, the direct object value found during matching
		prepstr   a string, the prepositional phrase found during parsing
		iobjstr   a string, the indirect object string
		iobj      an object, the indirect object value
		*/

		// The owner of a verb also determines the permissions with which that
		// verb runs; that is, the program in a verb can do whatever operations
		// the owner of that verb is allowed to do and no others.

		T.setPermissions( FndVrb->owner() );

		T.setObject( FndVrb->object() );

		setPermissions( T.permissions() );

		taskDump( "execute-verbCall", T );

		return( verbCallCode( FndVrb ) );
	}
	catch( const mooException &e )
	{
		setException( e );
	}
	catch( ... )
	{
	}

	return( lua_pushexception( lua_gettop( mL ) ) );
}

int lua_task::verbCall( ObjectId pObjectId, Verb *V, int pArgCnt )
{
	Task		T = task();

	lua_task::taskDump( QString( "verbCall( %1, %2, %3 )" ).arg( pObjectId ).arg( V->name() ).arg( pArgCnt ), T );

	T.setCaller( T.object() );
	T.setObject( V->object() );

	if( T.verb().isEmpty() )
	{
		T.setVerb( V->name() );
	}

	return( verbCall( T, V, pArgCnt ) );
}

int lua_task::verbCall( Task &pTask, Verb *V, int pArgCnt  )
{
	int			Result;

	lua_task::taskDump( "verbCall()", pTask );

	taskPush( pTask );

	Result = verbCallCode( V, pArgCnt );

	taskPop();

	return( Result );
}

int lua_task::verbCallCode( Verb *V, int pArgCnt )
{
	Connection		*C    = ConnectionManager::instance()->connection( connectionId() );

	int				c1    = lua_gettop( mL ) - pArgCnt;
	int				Error = V->lua_pushverb( mL );

	if( !Error )
	{
//		std::cout << "verbCallCode " << V->name().toStdString() << " with " << pArgCnt << " args (before args):" << std::endl;

//		lua_moo::stackReverseDump( mL );

		if( pArgCnt )
		{
			lua_insert( mL, -1 - pArgCnt );
		}

//		std::cout << "verbCallCode " << V->name().toStdString() << " with " << pArgCnt << " args (before call):" << std::endl;

//		lua_moo::stackReverseDump( mL );

		if( ( Error = lua_pcall( mL, pArgCnt, LUA_MULTRET, 0 ) ) == 0 )
		{

		}

//		std::cout << "verbCallCode " << V->name().toStdString() << " with " << pArgCnt << " args (after):" << std::endl;

//		lua_moo::stackReverseDump( mL );
	}

	if( Error )
	{
		QString		S = QString::fromLatin1( lua_tostring( mL, -1 ) );

		if( C )
		{
			C->notify( S.toHtmlEscaped() );
		}

		std::cerr << "verbCallCode" << S.toStdString() << std::endl;

		lua_pop( mL, 1 );

		mException = mooException( E_EXCEPTION, S );
	}

	int				c2 = lua_gettop( mL );

	int				ResCnt = c2 - c1;

//	qDebug() << "verbCallCode " << V->name() << "with" << ResCnt << "results (after):";

//	lua_moo::stackReverseDump( mL );

	return( ResCnt );
}

void lua_task::taskDump( const QString &S, const Task &T )
{
#if defined( QT_DEBUG ) && defined( MOO_DEBUG_TASKS )
	if( true )
#else
	if( false )
#endif
	{
		qDebug() << S << "id:" << T.id()
				 << "plr:" << T.player()
				 << "obj:" << T.object()
				 << "clr:" << T.caller()
				 << "prm:" << T.permissions()
				 << "argstr:" << T.argstr()
				 << "cmd:" << T.command()
				 << "elv:" << elevated()
				 << "verb:" << T.verb();
	}
}

int lua_task::lua_pushexception( int pRetVal )
{
	if( mException.error() != E_NONE )
	{
		mException.lua_pushexception( L() );

		return( lua_error( L() ) );
	}

	return( pRetVal );
}

void lua_task::taskPush( const Task &T )
{
	if( mTasks.size() >= 50 )
	{
		throw mooException( E_MAXREC, "task nest limit reached" );
	}

	mTasks.push_front( T );

	setPermissions( T.mPermissions );
}

void lua_task::taskPop()
{
#if defined( QT_DEBUG )
	if( false )
	{
		const Task &T = mTasks.front();

		qDebug() << "taskPop(" << T.id() << ")";
	}
#endif

	mTasks.pop_front();

	if( !mTasks.isEmpty() )
	{
		setPermissions( mTasks.front().mPermissions );
	}
}

void lua_task::luaHook( lua_State *L, lua_Debug *ar )
{
	if( ar->event == LUA_HOOKCOUNT )
	{
		lua_task		*T = lua_task::luaGetTask( L );

		if( T )
		{
			if( QDateTime::currentMSecsSinceEpoch() - T->timestamp() > 5000 )
			{
				//luaL_error( L, "function is taking too long" );

				//qDebug() << "function is taking too long";
			}
		}
	}
}

int lua_task::throwError( mooError pError, const QString pMessage )
{
	Q_UNUSED( pError )
	Q_UNUSED( pMessage )

	//qDebug() << "error:" << pError << pMessage;

	//throw( mooException( pError, pMessage ) );

	return( 0 );
}

QStringList lua_task::taskVerbStack() const
{
	QStringList		VrbLst;

	for( const Task &T : mTasks )
	{
		QString		VrbStr = T.verb();

		if( !VrbStr.isNull() )
		{
			VrbLst.prepend( VrbStr );
		}
	}

	return( VrbLst );
}

int lua_task::process( QString pCommand, ConnectionId pConnectionId, ObjectId pPlayerId, bool pElevated )
{
	lua_task		 Com( pConnectionId, TaskEntry( pCommand, pConnectionId, pPlayerId ) );
	int				 Ret = 0;

	Com.setElevated( pElevated );

	try
	{
		Ret = Com.eval();
	}
	catch( ... )
	{
	}

	return( Ret );
}

bool lua_task::isWizard() const
{
	ObjectId	 I = permissions();

	if( I == OBJECT_NONE )
	{
		return( true );
	}

	if( !mElevated )
	{
		return( false );
	}

	Object		*O = ObjectManager::o( I );

	return( O && O->wizard() );
}

bool lua_task::isProgrammer() const
{
	ObjectId	 I = permissions();

	if( I == OBJECT_NONE )
	{
		return( true );
	}

	Object		*O = ObjectManager::o( permissions() );

	return( O && O->programmer() );
}

bool lua_task::isOwner( ObjectId pObjectId ) const
{
	return( pObjectId == permissions() );
}

bool lua_task::isPermValid() const
{
	return( permissions() == OBJECT_NONE || ObjectManager::o( permissions() ) );
}

bool lua_task::isOwner( Object *O ) const
{
	return( O ? isOwner( O->owner() ) : false );
}

bool lua_task::isOwner(Verb *V) const
{
	return( isOwner( V->owner() ) );
}

bool lua_task::isOwner( Property *P ) const
{
	return( isOwner( P->owner() ) );
}
