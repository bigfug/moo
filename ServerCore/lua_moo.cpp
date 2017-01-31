#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_prop.h"
#include "lua_verb.h"
#include "lua_osc.h"
#include "lua_json.h"
#include "lua_serialport.h"
#include "lua_smtp.h"

#include "connectionmanager.h"

LuaMap		lua_moo::mLuaFun;
LuaMap		lua_moo::mLuaGet;
LuaMap		lua_moo::mLuaSet;

QNetworkAccessManager	lua_moo::mNAM;

const luaL_Reg lua_moo::mLuaGlobal[] =
{
	{ 0, 0 }
};

const luaL_Reg lua_moo::mLuaMeta[] =
{
	{ "__index", lua_moo::luaGet },
	{ "__newindex", lua_moo::luaSet },
	{ 0, 0 }
};

const luaL_Reg lua_moo::mLuaStatic[] =
{
	{ "checkpoint", lua_moo::luaCheckPoint },
	{ "notify", lua_moo::luaNotify },
	{ "pass", lua_moo::luaPass },
	{ "eval", lua_moo::luaEval },
	{ "debug", lua_moo::luaDebug },
	{ "hash", lua_moo::luaHash },
	{ "findPlayer", lua_moo::luaFindPlayer },
	{ "get", lua_moo::luaNetworkGet },
	{ 0, 0 }
};

const luaL_Reg lua_moo::mLuaGetFunc[] =
{
	{ "root", lua_moo::luaRoot },
	{ "last", lua_moo::luaLastObject },
	{ "timestamp", lua_moo::luaTimestamp },
	{ 0, 0 }
};

void lua_moo::initialise()
{
	addFunctions( mLuaStatic );
	addGet( mLuaGetFunc );
}

void lua_moo::addFunctions( const luaL_Reg *pFuncs )
{
	const luaL_Reg *FP = pFuncs;

	for( FP = pFuncs ; FP->name != 0 ; FP++ )
	{
		QString		FN = QString( FP->name );

		Q_ASSERT( mLuaFun.find( FN ) == mLuaFun.end() );

		mLuaFun[ FN ] = FP->func;
	}
}

void lua_moo::addGet( const luaL_Reg *pFuncs )
{
	const luaL_Reg *FP = pFuncs;

	for( FP = pFuncs ; FP->name != 0 ; FP++ )
	{
		QString		FN = QString( FP->name );

		Q_ASSERT( mLuaGet.find( FN ) == mLuaGet.end() );

		mLuaGet[ FN ] = FP->func;
	}
}

void lua_moo::addSet( const luaL_Reg *pFuncs )
{
	const luaL_Reg *FP = pFuncs;

	for( FP = pFuncs ; FP->name != 0 ; FP++ )
	{
		QString		FN = QString( FP->name );

		Q_ASSERT( mLuaSet.find( FN ) == mLuaSet.end() );

		mLuaSet[ FN ] = FP->func;
	}
}

void lua_moo::initialiseAll()
{
	lua_moo::initialise();
	lua_task::initialise();
	lua_object::initialise();
	lua_connection::initialise();
	lua_prop::initialise();
	lua_verb::initialise();
	lua_osc::initialise();
	lua_json::initialise();
	lua_serialport::initialise();
	lua_smtp::initialise();
}

void lua_moo::luaRegisterAllStates(lua_State *L)
{
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_moo::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_task::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_object::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_connection::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_verb::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_prop::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_osc::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_json::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_serialport::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_smtp::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );
}

void lua_moo::luaNewState( lua_State *L )
{
	luaL_openlibs( L );

//	luaopen_base( L );		lua_pop( L, 1 );
//	luaopen_table( L );		lua_pop( L, 1 );
//	luaopen_string( L );	lua_pop( L, 1 );
//	luaopen_math( L );		lua_pop( L, 1 );

	luaRegisterAllStates( L );
}

void lua_moo::luaRegisterState( lua_State *L )
{
	luaL_openlib( L, "moo", mLuaGlobal, 0 );

	luaL_newmetatable( L, "moo" );

	luaL_openlib( L, 0, mLuaMeta, 0 );

	lua_setmetatable( L, -2 );

	lua_pop( L, 1 );

	Q_ASSERT( lua_gettop( L ) == 0 );
}

void lua_moo::luaSetEnv( lua_State *L )
{
	lua_newtable( L );

	//--------------------------------------------------

	lua_getglobal( L, "assert" );
	lua_setfield( L, -2, "assert" );

	lua_getglobal( L, "ipairs" );
	lua_setfield( L, -2, "ipairs" );

	lua_getglobal( L, "next" );
	lua_setfield( L, -2, "next" );

	lua_getglobal( L, "pairs" );
	lua_setfield( L, -2, "pairs" );

	lua_getglobal( L, "pcall" );
	lua_setfield( L, -2, "pcall" );

	lua_getglobal( L, "select" );
	lua_setfield( L, -2, "select" );

	lua_getglobal( L, "tonumber" );
	lua_setfield( L, -2, "tonumber" );

	lua_getglobal( L, "tostring" );
	lua_setfield( L, -2, "tostring" );

	lua_getglobal( L, "type" );
	lua_setfield( L, -2, "type" );

	lua_getglobal( L, "unpack" );
	lua_setfield( L, -2, "unpack" );

	lua_getglobal( L, "_VERSION" );
	lua_setfield( L, -2, "_VERSION" );

	lua_getglobal( L, "xpcall" );
	lua_setfield( L, -2, "xpcall" );

	//--------------------------------------------------

	lua_getglobal( L, "table" );
	lua_setfield( L, -2, "table" );

	lua_getglobal( L, "string" );
	lua_setfield( L, -2, "string" );

	lua_getglobal( L, "math" );
	lua_setfield( L, -2, "math" );

	//--------------------------------------------------

	lua_getglobal( L, "moo" );
	lua_setfield( L, -2, "moo" );

	lua_getglobal( L, "o" );
	lua_setfield( L, -2, "o" );

	//--------------------------------------------------

	lua_newtable( L );
	lua_pushcfunction( L, lua_moo::luaGlobalIndex );
	lua_setfield( L, -2, "__index" );

	lua_setmetatable( L, -2 );

	lua_setfenv( L, -2 );
}

int lua_moo::luaGlobalIndex( lua_State *L )
{
	luaL_checktype( L, 1, LUA_TTABLE );
	luaL_checktype( L, 2, LUA_TSTRING );

	QString		 s( lua_tostring( L, 2 ) );

	lua_task			*lt = lua_task::luaGetTask( L );
	const Task			&t  = lt->task();
	QList<ObjectId>		 l;

	t.findObject( s, l );

	if( l.size() == 0 )
	{
		lua_pushnil( L );
	}
	else if( l.size() > 1 )
	{
		lua_newtable( L );

		for( int i = 0 ; i < l.size() ; i++ )
		{
			lua_pushinteger( L, i + 1 );
			lua_object::lua_pushobjectid( L, l[ i ] );
			lua_settable( L, -3 );
		}
	}
	else
	{
		lua_object::lua_pushobjectid( L, l.first() );
	}

	return( 1 );
}

int lua_moo::luaGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		QString				 N = QString( luaL_checkstring( L, 2 ) );

		// Look for function in mLuaFun

		lua_CFunction	 F;

		if( ( F = mLuaFun.value( N, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		if( ( F = mLuaGet.value( N, 0 ) ) != 0 )
		{
			return( F( L ) );
		}
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaSet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		QString				 N = QString( luaL_checkstring( L, 2 ) );

		// Look for function in mLuaFun

		lua_CFunction	 F;

		if( ( F = mLuaSet.value( N, 0 ) ) != 0 )
		{
			return( F( L ) );
		}
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaNotify( lua_State *L )
{
	bool				 LuaErr = false;
	int					 ArgCnt = lua_gettop( L );

	if( ArgCnt <= 0 )
	{
		return( 0 );
	}

	QString				 Msg;

	for( int i = 0 ; i < ArgCnt ; i++ )
	{
		int		ArgIdx = -1 - i;

		switch( lua_type( L, ArgIdx ) )
		{
			case LUA_TSTRING:
				Msg.append( lua_tostring( L, ArgIdx ) );
				break;

			case LUA_TNIL:
				Msg.append( "<nil>" );
				break;

			case LUA_TBOOLEAN:
				if( lua_toboolean( L, ArgIdx ) )
				{
					Msg.append( "true" );
				}
				else
				{
					Msg.append( "false" );
				}
				break;

			case LUA_TNUMBER:
				Msg.append( QString( "%1" ).arg( lua_tonumber( L, ArgIdx ) ) );
				break;

			case LUA_TUSERDATA:
				break;
		}
	}

	if( Msg.isEmpty() )
	{
		return( 0 );
	}

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionid() );

		if( C != 0 )
		{
			C->notify( Msg );
		}

		return( 0 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaRoot( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		Object		*O = ObjectManager::instance()->object( 0 );

		if( O == 0 )
		{
			throw( mooException( E_INVARG, "root object has not been defined yet" ) );
		}

		lua_object::lua_pushobject( L, O );

		return( 1 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

// pass calls the verb with the same name as the current verb but as defined on
//   the parent of the object that defines the current verb

int lua_moo::luaPass( lua_State *L )
{
	Q_UNUSED( L )

//	Task			*T = lua_task::luaGetTask( L );
//	ObjectManager	&OM = *ObjectManager::instance();
//	ObjectId		 id = T.object();

//	while( id != -1 )
//	{
//		Object			*O = OM.object( id );

//		if( O == 0 )
//		{
//			return( 0 );
//		}

//		Object			*P = OM.object( O->parent() );

//		if( P == 0 )
//		{
//			return( 0 );
//		}

//		if( P->verb( T.string() ) != 0 )
//		{
//			return( lua_object::verbCall( T, P, T.string(), lua_gettop( L ) - 1 ) );
//		}

//		id = O->parent();
//	}

	return( 0 );
}

int lua_moo::luaEval( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		const char			*C = luaL_checkstring( L, -1 );
		Task				 E( C );
		int					 Results;
		Object				*PRG = ObjectManager::o( T.programmer() );

		if( PRG == 0 || !PRG->programmer() )
		{
			throw mooException( E_PERM, "programmer is not a programmer!" );
		}

		E.setProgrammer( T.programmer() );
		E.setPlayer( T.player() );
		E.setCaller( T.object() );
		E.setObject( OBJECT_NONE );

		Command->taskPush( E );

		Results = Command->eval();

		Command->taskPop();

		return( Results );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaHash(lua_State *L)
{
	const char	*S = luaL_checkstring( L, -1 );

	QByteArray	 H = QCryptographicHash::hash( S, QCryptographicHash::Sha256 );

	lua_pushstring( L, H.toBase64() );

	return( 1 );
}

int lua_moo::luaDebug( lua_State *L )
{
	for( int i = 1 ; i <= lua_gettop( L ) ; i++ )
	{
		qDebug() << QString( lua_tostring( L, i ) );
	}

	return( 0 );
}

int lua_moo::luaFindPlayer( lua_State *L )
{
	const char				*S = luaL_checkstring( L, -1 );

	ObjectManager			&OM = *ObjectManager::instance();
	ObjectId				 PID = OM.findPlayer( QString::fromLatin1( S ) );

	if( PID == OBJECT_NONE )
	{
		lua_pushnil( L );
	}
	else
	{
		lua_object::lua_pushobjectid( L, PID );
	}

	return( 1 );
}

int lua_moo::luaTimestamp(lua_State *L)
{
	lua_pushnumber( L, lua_Number( ObjectManager::timestamp() ) / 1000.0 );

	return( 1 );
}

int lua_moo::luaCheckPoint( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*PRG = ObjectManager::o( T.programmer() );

		if( PRG == 0 || !PRG->wizard() )
		{
			throw mooException( E_PERM, "wizard is not a wizard!" );
		}

		ObjectManager			&OM = *ObjectManager::instance();

		OM.checkpoint();
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );

}

int lua_moo::luaLastObject(lua_State *L)
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionid() );
		Object				*O = ObjectManager::instance()->object( C->lastCreatedObjectId() );

		if( !O )
		{
			throw( mooException( E_INVARG, "no last object" ) );
		}

		lua_object::lua_pushobject( L, O );

		return( 1 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaNetworkGet( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		const char			*R = luaL_checkstring( L, 1 );
		Object				*O = lua_object::argObj( L, 2 );
		const char			*V = luaL_checkstring( L, 3 );

		Object				*PRG = ObjectManager::o( T.programmer() );

		if( !PRG || !PRG->wizard() )
		{
			throw mooException( E_PERM, "only wizards can make network requests" );
		}

		if( !O )
		{
			throw( mooException( E_INVARG, "no object" ) );
		}

		if( !O->verb( V ) )
		{
			throw( mooException( E_INVARG, "no verb on object" ) );
		}

		QUrl			 NetUrl( R );

		if( !NetUrl.isValid() )
		{
			throw( mooException( E_INVARG, "invalid URL" ) );
		}

		QNetworkRequest	 NetReq( NetUrl );

		QNetworkReply	*NetRep = mNAM.get( NetReq );

		if( !NetRep )
		{
			throw( mooException( E_MEMORY, "can't make network request" ) );
		}

		NetRep->setProperty( "oid", O->id() );
		NetRep->setProperty( "verb", QString::fromLatin1( V ) );

		QObject::connect( NetRep, SIGNAL(readyRead()), ObjectManager::instance(), SLOT(networkRequestReadyRead()) );
		QObject::connect( NetRep, SIGNAL(finished()), ObjectManager::instance(), SLOT(networkRequestFinished()) );

		return( 0 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_moo::luaPanic( lua_State *L )
{
	Q_UNUSED( L )

	qDebug() << "**** LUA PANIC ****";

	return( 0 );
}
