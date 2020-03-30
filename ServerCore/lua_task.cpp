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
	{ "programmer", lua_task::luaGetProgrammer },
	{ 0, 0 }
};

const luaL_Reg lua_task::mLuaSet[] =
{
	{ "programmer", lua_task::luaSetProgrammer },
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
	const Task			&T = lua_task::luaGetTask( L )->task();

	luaL_checkinteger( L, 1 );
	luaL_checkstring( L, 2 );

	lua_task			*lt = lua_task::luaGetTask( L );

	TaskEntry			E( QString( lua_tostring( L, 2 ) ), lt->connectionId(), T.programmer() );

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

int lua_task::luaGetProgrammer( lua_State *L )
{
	const lua_task			*LT = lua_task::luaGetTask( L );

	lua_object::lua_pushobjectid( L, LT->programmer() );

	return( 1 );
}

int lua_task::luaSetProgrammer( lua_State *L )
{
	lua_task			*LT = lua_task::luaGetTask( L );
	ObjectId			 WID = lua_object::argId( L, -1 );
	Object				*PRG = ObjectManager::o( LT->programmer() );

	if( PRG->id() == WID || PRG->wizard() )
	{
		LT->setProgrammer( WID );
	}
	else
	{
		luaL_error( L, "bad permissions" );
	}

	return( 0 );
}

lua_task::lua_task( ConnectionId pConnectionId, const Task &pTask )
	: mL( Q_NULLPTR ), mConnectionId( pConnectionId ), mTimeStamp( 0 ), mMemUse( 0 ), mError( false )
{
	mTasks.push_front( pTask );
}

lua_task::lua_task( lua_task &&t )
	: mL( std::move( t.mL ) ),
	  mConnectionId( std::move( t.mConnectionId ) ),
	  mTasks( std::move( t.mTasks ) ),
	  mTimeStamp( std::move( t.mTimeStamp ) ),
	  mMemUse( std::move( t.mMemUse ) ),
	  mChanges( std::move( t.mChanges ) ),
	  mError( std::move( t.mError ) )
{
	lua_setallocf( mL, lua_task::luaAlloc, this );

	luaSetTask( mL, this );

	t.mL = nullptr;
	t.mConnectionId = CONNECTION_NONE;
	t.mMemUse = 0;
	t.mError  = false;
	t.mTimeStamp = 0;
}

lua_task::~lua_task( void )
{
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

	if( !mError )
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

		mTimeStamp = QDateTime::currentMSecsSinceEpoch();

		lua_moo::luaNewState( mL );

		lua_sethook( mL, &lua_task::luaHook, LUA_MASKCOUNT, 10 );

		luaSetTask( mL, this );
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

	mTimeStamp = pTimeStamp;

	// the server next checks to see if the first word names any of the six "built-in" commands:
	// `.program', `PREFIX', `OUTPUTPREFIX', `SUFFIX', `OUTPUTSUFFIX',
	// or the connection's defined flush command, if any (`.flush' by default).

	T.setCaller( T.player() );
	T.setVerb( First );
	T.setArgStr( ArgStr );
	T.setArgs( Words );

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

	if( ( Error = luaL_loadstring( LS, T.command().toLatin1() ) ) == 0 )
	{
		lua_getglobal( LS, "moo_sandbox" );

		lua_setupvalue( LS, -2, 1 );

		if( ( Error = lua_pcall( LS, 0, LUA_MULTRET, 0 ) ) == 0 )
		{

		}
	}

	if( Error )
	{
		QString		Err = QString( lua_tostring( LS, -1 ) );

		Connection	*CON = ConnectionManager::instance()->connection( connectionId() );

		if( CON )
		{
			CON->notify( Err );
		}

		std::cerr << "eval" << Err.toStdString() << std::endl;

		lua_pop( LS, 1 );

		mError = true;
	}

	const int			 NewTop = lua_gettop( LS );

	return( NewTop - OldTop );
}

int lua_task::subeval()
{
	return( 0 );
}

int lua_task::executeLogin( void )
{
	bool		LuaErr = false;

	try
	{
		Task			&T				= mTasks.front();
		ObjectManager	&OM				= *ObjectManager::instance();
		Object			*System			= OM.systemObject();
		Verb			*LoginCommand	= ( System ? System->verbMatch( "do_login_command" ) : Q_NULLPTR );
		ObjectId		 MaxId			= OM.maxId();
		Verb			*V;

		if( !LoginCommand )
		{
			return( 0 );
		}

		T.setProgrammer( System->owner() );

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

		ConnectionManager			&CM = *ConnectionManager::instance();
		const ConnectionNodeMap		&NM = CM.connectionList();
		bool						 UR = false;

		for( ConnectionNodeMap::const_iterator it = NM.begin() ; it != NM.end() ; it++ )
		{
			Connection		*C = it.value();

			if( C->player() == Player->id() )
			{
				if( C->object() != 0 )
				{
					if( ( V = System->verbMatch( "user_client_disconnected" ) ) != Q_NULLPTR )
					{
						lua_object::lua_pushobject( mL, Player );

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

		CM.logon( connectionId(), Player->id() );

		Player->setConnection( connectionId() );

		T.setPlayer( Player->id() );

		if( OM.maxId() > MaxId )
		{
			if( ( V = System->verbMatch( "user_created" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, Player );

				verbCall( System->id(), V, 1 );
			}
		}
		else if( UR )
		{
			if( ( V = System->verbMatch( "user_reconnected" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, Player );

				verbCall( System->id(), V, 1 );
			}
		}
		else
		{
			if( ( V = System->verbMatch( "user_connected" ) ) != 0 )
			{
				lua_object::lua_pushobject( mL, Player );

				verbCall( System->id(), V, 1 );
			}
		}
	}
	catch( mooException &e )
	{
		e.lua_pushexception( mL );

		LuaErr = true;
	}
	catch( ... )
	{
	}

	return( LuaErr ? lua_error( mL ) : lua_gettop( mL ) );
}

int lua_task::execute( void )
{
	bool		LuaErr = false;

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

		T.setProgrammer( FndVrb->owner() );

		return( verbCallCode( FndVrb ) );
	}
	catch( mooException &e )
	{
		e.lua_pushexception( mL );

		LuaErr = true;
	}
	catch( ... )
	{
	}

	return( LuaErr ? lua_error( mL ) : lua_gettop( mL ) );
}

int lua_task::verbCall( ObjectId pObjectId, Verb *V, int pArgCnt )
{
	Task		T = task();

	T.setProgrammer( V->owner() );
	T.setCaller( T.object() );
	T.setObject( pObjectId );

	if( T.verb().isEmpty() )
	{
		T.setVerb( V->name() );
	}

	return( verbCall( T, V, pArgCnt ) );
}

int lua_task::verbCall( Task &pTask, Verb *V, int pArgCnt  )
{
	int			Result;

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
//		qDebug() << "verbCallCode " << V->name() << "with" << pArgCnt << " args (before args):";

//		lua_moo::stackReverseDump( mL );

		if( pArgCnt )
		{
			lua_insert( mL, -1 - pArgCnt );
		}

//		qDebug() << "verbCallCode " << V->name() << "with" << pArgCnt << " args (before):";

//		lua_moo::stackReverseDump( mL );

		if( ( Error = lua_pcall( mL, pArgCnt, LUA_MULTRET, 0 ) ) == 0 )
		{

		}
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

		mError = true;
	}

	int				c2 = lua_gettop( mL );

	int				ResCnt = c2 - c1;

//	qDebug() << "verbCallCode " << V->name() << "with" << ResCnt << "results (after):";

//	lua_moo::stackReverseDump( mL );

	return( ResCnt );
}


void lua_task::taskPush( const Task &T )
{
	if( mTasks.size() >= 50 )
	{
		throw mooException( E_MAXREC, "task nest limit reached" );
	}

	mTasks.push_front( T );
}

void lua_task::taskPop()
{
	mTasks.pop_front();
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

int lua_task::process( QString pCommand, ConnectionId pConnectionId, ObjectId pPlayerId )
{
	lua_task		 Com( pConnectionId, TaskEntry( pCommand, pConnectionId, pPlayerId ) );
	int				 Ret = 0;

	try
	{
		Ret = Com.eval();
	}
	catch( ... )
	{
	}

	return( Ret );
}
