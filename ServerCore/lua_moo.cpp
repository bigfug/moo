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
#include "lua_text.h"

#include "inputsinkread.h"

#include "connectionmanager.h"

#include "changeset/connectionnotify.h"
#include "changeset/connectionsetcookie.h"
#include "changeset/connectionclearcookie.h"

LuaMap		lua_moo::mLuaFun;
LuaMap		lua_moo::mLuaGet;
LuaMap		lua_moo::mLuaSet;

QNetworkAccessManager	lua_moo::mNAM;

QString		lua_moo::mSettingsFilePath = "moo.ini";

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
	{ "broadcast", lua_moo::luaBroadcast },
	{ "checkpoint", lua_moo::luaCheckPoint },
	{ "notify", lua_moo::luaNotify },
	{ "pass", lua_moo::luaPass },
	{ "eval", lua_moo::luaEval },
	{ "elevate", lua_moo::luaElevate },
	{ "debug", lua_moo::luaDebug },
	{ "hash", lua_moo::luaHash },
	{ "findPlayer", lua_moo::luaFindPlayer },
	{ "findByProp", lua_moo::luaFindByProp },
	{ "find_player", lua_moo::luaFindPlayer },
	{ "find_by_prop", lua_moo::luaFindByProp },
	{ "get", lua_moo::luaNetworkGet },
	{ "read", lua_moo::luaRead },
	{ "find", lua_moo::luaFind },
	{ "cookie", lua_moo::luaCookie },
	{ "setCookie", lua_moo::luaSetCookie },
	{ "set_cookie", lua_moo::luaSetCookie },
	{ "clear", lua_moo::luaClearCookie },
	{ "isValidObject", lua_moo::luaIsValidObject },
	{ "is_valid_object", lua_moo::luaIsValidObject },
	{ "gmcp", lua_moo::luaGMCP },
	{ "import", lua_moo::luaImport },
	{ 0, 0 }
};

const luaL_Reg lua_moo::mLuaGetFunc[] =
{
	{ "root", lua_moo::luaRoot },
	{ "last", lua_moo::luaLastObject },
	{ "system", lua_moo::luaSystem },
	{ "timestamp", lua_moo::luaTimestamp },
	{ 0, 0 }
};

int luaTableIndexOf( lua_State *L )
{
	luaL_checktype( L, 1, LUA_TTABLE );
	luaL_checkany( L, 2 );

	lua_pushvalue( L, 2 );

	size_t				 SrcLen;
	const char			*SrcDat = lua_tolstring( L, -1, &SrcLen );

	const QString		 SrcStr( SrcDat );

	lua_pop( L, 1 );

	// iterate over table

	QVariant			 DstKey;

	lua_pushvalue( L, 1 );		// stack now contains: -1 => table

	lua_pushnil( L );			// stack now contains: -1 => nil; -2 => table

	while( DstKey.isNull() && lua_next( L, -2 ) )
	{
		// stack now contains: -1 => value; -2 => key; -3 => table

		lua_pushvalue( L, -1 );	// stack now contains: -1 => value; -2 => value; -3 => key; -4 => table

		const char *value = lua_tostring( L, -2 );	// may convert

		if( !SrcStr.compare( QString( value ) ) )
		{
			switch( lua_type( L, -3 ) )
			{
				case LUA_TSTRING:
					DstKey = QString( luaL_optstring( L, -3, NULL ) );
					break;

				case LUA_TNUMBER:
					DstKey = luaL_optinteger( L, -3, 0 );
					break;
			}
		}

		lua_pop( L, 2 );		// stack now contains: -1 => key; -2 => table
	}

	lua_pop( L, 1 );			// pop table

	luaL_pushvariant( L, DstKey );

	return( 1 );
}

const luaL_Reg luaTableFuncs[] =
{
	{ "indexOf", luaTableIndexOf },
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
	lua_text::initialise();
}

void lua_moo::luaNewState( lua_State *L )
{
	lua_atpanic( L, lua_moo::luaPanic );

	// load all Lua libraries into global

	luaL_openlibs( L );

	luaRegisterAllStates( L );

	luaSetEnv( L );
}

void lua_moo::setSettingsFilePath( QString pFilePath )
{
	mSettingsFilePath = pFilePath;
}

QString lua_moo::settingsFilePath()
{
	return( mSettingsFilePath );
}

void lua_moo::luaRegisterAllStates(lua_State *L)
{
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_moo::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_object::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_task::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_connection::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_verb::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_prop::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_json::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_osc::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_serialport::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_smtp::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );

	lua_text::luaRegisterState( L );
	Q_ASSERT( lua_gettop( L ) == 0 );
}

lua_State *lua_moo::luaNewState( void )
{
	lua_State		*L = luaL_newstate();

	luaNewState( L );

	return( L );
}

void lua_moo::luaRegisterState( lua_State *L )
{
	Q_UNUSED( L )
}

void lua_moo::luaSetEnv( lua_State *L )
{
	lua_newtable( L );

	//--------------------------------------------------

	luaL_newmetatable( L, "moo" );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaMeta, 0 );

	lua_pop( L, 1 );

	//--------------------------------------------------

	lua_newtable( L );

	luaL_getmetatable( L, "moo" );
	lua_setmetatable( L, -2 );

	lua_setfield( L, -2, "moo" );

	//--------------------------------------------------

	lua_pushcfunction( L, lua_object::luaObject );
	lua_setfield( L, -2, "o" );

	//--------------------------------------------------

	lua_getglobal( L, "assert" );
	lua_setfield( L, -2, "assert" );

	lua_getglobal( L, "ipairs" );
	lua_setfield( L, -2, "ipairs" );

	lua_getglobal( L, "error" );
	lua_setfield( L, -2, "error" );

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

	lua_getglobal( L, "_VERSION" );
	lua_setfield( L, -2, "_VERSION" );

	lua_getglobal( L, "xpcall" );
	lua_setfield( L, -2, "xpcall" );

#if defined( QT_DEBUG )
	lua_getglobal( L, "print" );
	lua_setfield( L, -2, "print" );

	lua_getglobal( L, "getmetatable" );
	lua_setfield( L, -2, "getmetatable" );
#endif

	//--------------------------------------------------

	lua_getglobal( L, "table" );

//	luaL_register( L, NULL, luaTableFuncs );	// leaves table on stack

	lua_setfield( L, -2, "table" );

	lua_getglobal( L, "string" );
	lua_setfield( L, -2, "string" );

	lua_getglobal( L, "math" );
	lua_setfield( L, -2, "math" );

	//--------------------------------------------------

	lua_newtable( L );
	lua_getglobal( L, "os" );

	lua_getfield( L, -1, "time" );
	lua_setfield( L, -3, "time" );

	lua_getfield( L, -1, "date" );
	lua_setfield( L, -3, "date" );

	lua_getfield( L, -1, "clock" );
	lua_setfield( L, -3, "clock" );

	lua_getfield( L, -1, "difftime" );
	lua_setfield( L, -3, "difftime" );

	lua_pop( L, 1 );

	lua_setfield( L, -2, "os" );

	//--------------------------------------------------

	lua_object::lua_pushobjectid( L, OBJECT_NONE );
	lua_setfield( L, -2, "O_NONE" );

	lua_object::lua_pushobjectid( L, OBJECT_AMBIGUOUS );
	lua_setfield( L, -2, "O_AMBIGUOUS" );

	lua_object::lua_pushobjectid( L, OBJECT_FAILED_MATCH );
	lua_setfield( L, -2, "O_FAILED_MATCH" );

	lua_object::lua_pushobjectid( L, OBJECT_UNSPECIFIED );
	lua_setfield( L, -2, "O_UNSPECIFIED" );

	//--------------------------------------------------

	lua_pushinteger( L, E_NONE );
	lua_setfield( L, -2, "E_NONE" );

	lua_pushinteger( L, E_TYPE );
	lua_setfield( L, -2, "E_TYPE" );

	lua_pushinteger( L, E_DIV );
	lua_setfield( L, -2, "E_DIV" );

	lua_pushinteger( L, E_PERM );
	lua_setfield( L, -2, "E_PERM" );

	lua_pushinteger( L, E_PROPNF );
	lua_setfield( L, -2, "E_PROPNF" );

	lua_pushinteger( L, E_VERBNF );
	lua_setfield( L, -2, "E_VERBNF" );

	lua_pushinteger( L, E_VARNF );
	lua_setfield( L, -2, "E_VARNF" );

	lua_pushinteger( L, E_INVIND );
	lua_setfield( L, -2, "E_INVIND" );

	lua_pushinteger( L, E_RECMOVE );
	lua_setfield( L, -2, "E_RECMOVE" );

	lua_pushinteger( L, E_MAXREC );
	lua_setfield( L, -2, "E_MAXREC" );

	lua_pushinteger( L, E_RANGE );
	lua_setfield( L, -2, "E_RANGE" );

	lua_pushinteger( L, E_ARGS );
	lua_setfield( L, -2, "E_ARGS" );

	lua_pushinteger( L, E_NACC );
	lua_setfield( L, -2, "E_NACC" );

	lua_pushinteger( L, E_VERBNF );
	lua_setfield( L, -2, "E_VERBNF" );

	lua_pushinteger( L, E_INVARG );
	lua_setfield( L, -2, "E_INVARG" );

	lua_pushinteger( L, E_QUOTA );
	lua_setfield( L, -2, "E_QUOTA" );

	lua_pushinteger( L, E_FLOAT );
	lua_setfield( L, -2, "E_FLOAT" );

	//--------------------------------------------------

//	stackDump( L );

	lua_setglobal( L, "moo_sandbox" );
}

int lua_moo::luaGlobalIndex( lua_State *L )
{
	luaL_checktype( L, 1, LUA_TTABLE );
	luaL_checktype( L, 2, LUA_TSTRING );

	QString		 s( lua_tostring( L, 2 ) );

	qDebug() << "luaGlobalIndex:" << s;

	lua_task			*lt = lua_task::luaGetTask( L );
	const Task			&t  = lt->task();
	QList<ObjectId>		 l;

	t.findObject( s, l );

	if( l.isEmpty() )
	{
		lua_pushnil( L );
	}
	else if( l.size() == 1 )
	{
		lua_object::lua_pushobjectid( L, l.first() );
	}
	else
	{
		lua_newtable( L );

		for( int i = 0 ; i < l.size() ; i++ )
		{
			lua_pushinteger( L, i + 1 );
			lua_object::lua_pushobjectid( L, l[ i ] );
			lua_settable( L, -3 );
		}
	}

	return( 1 );
}

int lua_moo::luaGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_moo::luaSet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

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
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_moo::luaBroadcast( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		ConnectionManager	*CM = ConnectionManager::instance();

		if( !Command->isWizard() )
		{
			throw mooException( E_PERM, "only wizards can broadcast" );
		}

		QString				 Msg;

		for( Connection *C : CM->connectionList().values() )
		{
			if( C )
			{
				Object				*O  = ObjectManager::o( C->player() );

				Msg = lua_text::processString( L, O, 1 );

				Command->changeAdd( new change::ConnectionNotify( C, Msg ) );
			}
		}

		return( 0 );
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

int lua_moo::luaNotify( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( C )
		{
			Object				*O  = ObjectManager::o( Command->task().player() );
			QString				 Msg;

			Msg = lua_text::processString( L, O, 1 );

			Command->changeAdd( new change::ConnectionNotify( C, Msg ) );
		}

		return( 0 );
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

int lua_moo::luaRoot( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object		*O = ObjectManager::instance()->rootObject();

		if( !O )
		{
			throw( mooException( E_INVARG, "root object has not been defined yet" ) );
		}

		lua_object::lua_pushobject( L, O );

		return( 1 );
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

int lua_moo::luaSystem( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Object		*O = ObjectManager::instance()->systemObject();

		if( !O )
		{
			throw( mooException( E_INVARG, "system object has not been defined yet" ) );
		}

		return( lua_object::lua_pushobject( L, O ) );
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

// pass calls the verb with the same name as the current verb but as defined on
//   the parent of the object that defines the current verb

int lua_moo::luaPass( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		const Task			&T = Command->task();
		ObjectManager		&OM = *ObjectManager::instance();
		ObjectId			 id = T.object();
		bool				 VrbFnd = false;
		Verb				*V;

		Command->taskDump( "luaPass()", T );

		Object				*O = OM.object( T.object() );
		Object				*P = OM.object( O->parent() );

		while( P )
		{
			Verb		*V = P->verb( T.verb() );

			if( V )
			{
				Task		PT = T;

				PT.setPermissions( V->owner() );

				qDebug() << "luaPass - obj:" << T.object() << "- vob:" << P->id() << "- vrb:" << T.verb();

				return( Command->verbCall( PT, V, lua_gettop( L ) ) );
			}

			P = OM.object( P->parent() );
		}
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

int lua_moo::luaEval( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		const Task			&T = Command->task();
		const char			*C = luaL_checkstring( L, -1 );
		Task				 E( C );
		int					 Results;

		if( !Command->isProgrammer() )
		{
			throw mooException( E_PERM, "programmer is not a programmer!" );
		}

		Command->taskDump( "luaEval( T )", T );

		E.setPermissions( T.permissions() );
		E.setPlayer( T.player() );
		E.setCaller( T.object() );

		Command->taskDump( "luaEval( E )", E );

		Command->taskPush( E );

		Results = Command->eval();

		Command->taskPop();

		return( Results );
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

int lua_moo::luaElevate( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	const char			*ArgStr = luaL_checkstring( L, 1 );

	try
	{
		const Task          &T  = Command->task();
		Object				*O  = ObjectManager::o( Command->permissions() );

		if( Command->permissions() != OBJECT_NONE && ( !O || !O->wizard() ) )
		{
			throw mooException( E_PERM, "programmer is not a wizard!" );
		}

//		Connection            *C = ConnectionManager::instance()->connection( LT->connectionId() );

//        if( !T.elevated() )
//        {
//            if( C && LT->timestamp() - C->lastElevationTime() > 60000 )
//            {
//                InputSinkRead        *IS = new InputSinkRead( C, T, ReadOpts, VerbArgs );

//                if( IS )
//                {
//                    C->pushInputSink( IS );

//                    return( 0 );
//                }
//            }
//        }
//        else
		{
			Task                 E( ArgStr );

			E.setPermissions( T.player() );
			E.setPlayer( T.player() );
			E.setCaller( T.object() );
			E.setObject( OBJECT_NONE );

			bool	CurE = Command->elevated();

			Command->setElevated( true );

			Command->taskDump( "luaElevate()", E );

			Command->taskPush( E );

			int Results = Command->eval();

			Command->taskPop();

			Command->setElevated( CurE );

//            if( C )
//            {
//                C->setLastElevationTime( T.timestamp() );
//            }

			return( Results );
		}
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

int lua_moo::luaHash( lua_State *L )
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
		std::cout << lua_tostring( L, i ) << std::endl;
	}

	return( 0 );
}

int lua_moo::luaFindPlayer( lua_State *L )
{
	const char				*S = luaL_checkstring( L, -1 );

	ObjectManager			&OM = *ObjectManager::instance();

	lua_object::lua_pushobjectid( L, OM.findPlayer( QString::fromLatin1( S ) ) );

	return( 1 );
}

int lua_moo::luaFindByProp( lua_State *L )
{
	const char				*S = luaL_checkstring( L, 1 );

	luaL_checkany( L, 2 );

	ObjectManager			&OM = *ObjectManager::instance();
	QVariant				 V;

	switch( lua_type( L, 2 ) )
	{
		case LUA_TBOOLEAN:
			V = lua_toboolean( L, 2 );
			break;

		case LUA_TNUMBER:
			V = lua_tonumber( L, 2 );
			break;

		case LUA_TSTRING:
			V = QString::fromLatin1( lua_tostring( L, 2 ) );
			break;
	}

	lua_object::lua_pushobjectid( L, OM.findByProp( QString::fromLatin1( S ), V ) );

	return( 1 );
}

int lua_moo::luaFind( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	size_t					 StrLen;
	const char				*StrDat = luaL_checklstring( L, -1, &StrLen );
	ObjectId				 PID = OBJECT_NONE;

	if( StrDat && StrLen > 0 )
	{
		const QString			 S = QString::fromLatin1( StrDat, int( StrLen ) );

		const Task				&T = Command->task();
		QList<ObjectId>			 ObjLst;

		T.findObject( S, ObjLst );

		if( ObjLst.size() == 1 )
		{
			PID = ObjLst.first();
		}
		else if( ObjLst.size() > 1 )
		{
			PID = OBJECT_AMBIGUOUS;
		}
	}

	lua_object::lua_pushobjectid( L, PID );

	return( 1 );
}

int lua_moo::luaIsValidObject( lua_State *L )
{
	luaL_checkany( L, 1 );

	lua_object::luaHandle	*H = (lua_object::luaHandle *)luaL_testudata( L, 1, lua_object::luaHandle::mLuaName );

	lua_pushboolean( L, H && H->O >= 0 );

	return( 1 );
}

int lua_moo::luaTimestamp(lua_State *L)
{
	lua_pushnumber( L, lua_Number( ObjectManager::timestamp() ) / 1000.0 );

	return( 1 );
}

int lua_moo::luaCheckPoint( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		if( !Command->isWizard() )
		{
			throw mooException( E_PERM, "wizard is not a wizard!" );
		}

		ObjectManager::instance()->checkpoint();
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

int lua_moo::luaLastObject(lua_State *L)
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );
		Object				*O = ObjectManager::instance()->object( C->lastCreatedObjectId() );

		if( !O )
		{
			throw( mooException( E_INVARG, "no last object" ) );
		}

		lua_object::lua_pushobject( L, O );

		return( 1 );
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

int lua_moo::luaNetworkGet( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		const char			*R = luaL_checkstring( L, 1 );
		Object				*O = lua_object::argObj( L, 2 );
		const char			*V = luaL_checkstring( L, 3 );

		if( !Command->isWizard() )
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

int lua_moo::luaPanic( lua_State *L )
{
	Q_UNUSED( L )

	qDebug() << "**** LUA PANIC ****";

	throw std::runtime_error( "**** LUA PANIC ****" );

	return( 0 );
}

int lua_moo::luaGMCP( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );
	Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

	size_t		 PkgSze  = 0;
	const char	*Package = luaL_checklstring( L, 1, &PkgSze );

	luaL_checkany( L, 2 );

	QString		 Data;

	switch( lua_type( L, 2 ) )
	{
		case LUA_TBOOLEAN:
			{
				Data = lua_toboolean( L, 2 ) ? "true" : "false";
			}
			break;

		case LUA_TNUMBER:
			{
				Data = QString::number( lua_tonumber( L, 2 ) );
			}
			break;

		case LUA_TSTRING:
			{
				Data = QString::fromLatin1( lua_tostring( L, 2 ) );
			}
			break;

		case LUA_TTABLE:
			{
				QVariantMap		ValMap;

				lua_pushvalue( L, 2 );
				lua_pushnil( L );

				while( lua_next( L, -2 ) )
				{
					lua_pushvalue( L, -2 );

					const char *key = lua_tostring(L, -1);

					QVariant	V;

					switch( lua_type( L, -2 ) )
					{
						case LUA_TBOOLEAN:
							{
								V = lua_toboolean( L, -2 );
							}
							break;

						case LUA_TNUMBER:
							{
								V = lua_tonumber( L, -2 );
							}
							break;

						case LUA_TSTRING:
							{
								V = QString::fromLatin1( lua_tostring( L, -2 ) );
							}
							break;
					}

					ValMap.insert( QString::fromLatin1( key ), V );

					lua_pop( L, 2 );
				}

				lua_pop( L, 1 );


				if( true )
				{
					QJsonDocument	D( QJsonObject::fromVariantMap( ValMap ) );

					Data = D.toJson( QJsonDocument::Compact );
				}
			}
			break;
	}

	QByteArray		A = QByteArray::fromRawData( Package, PkgSze );

	if( !Data.isEmpty() )
	{
		A.append( ' ' );
		A.append( Data );
	}

	C->sendGMCP( A );

	return( 0 );
}

int lua_moo::luaImport( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );
	size_t				 FNSize;
	const char			*FNChars = luaL_checklstring( L, 1, &FNSize );

	try
	{
		Connection	*C = Command->connection();

		if( !Command->isWizard() )
		{
			throw mooException( E_PERM, "you have to be a wizard to do that!" );
		}

		QFile		 FH( QString::fromUtf8( FNChars, int( FNSize ) ) );

		if( !FH.open( QFile::ReadOnly | QFile::Text ) )
		{
			throw mooException( E_NACC, "import can't access that file" );
		}

		QString	HDR = FH.readLine().trimmed();

		if( !HDR.startsWith( "-- ArtMOO Script --" ) )
		{
			throw mooException( E_NACC, "import script doesn't begin with correct header" );
		}

		lua_task		ImportTask( Command->connectionId(), TaskEntry( "", Command->connectionId(), Command->permissions() ) );

		while( !FH.atEnd() )
		{
			QString	FL = FH.readLine().trimmed();

			if( FL.isEmpty() )
			{
				continue;
			}

			C->notify( FL.toHtmlEscaped() );

			ImportTask.taskPush( TaskEntry( FL, Command->connectionId(), Command->permissions() ) );

			ImportTask.execute( Command->timestamp() );

			ImportTask.taskPop();
		}

		FH.close();

		C->notify( "-- Import Complete" );
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

QVariantMap lua_moo::parseReadArgs( lua_State *L, int pIndex )
{
	QVariantMap ReadOpts;

	luaL_checktype( L, pIndex, LUA_TTABLE );

	lua_pushvalue( L, pIndex );

	lua_pushnil( L );

	// stack now contains: -1 => nil; -2 => table

	while( lua_next( L, -2 ) )
	{
		// stack now contains: -1 => value; -2 => key; -3 => table

		// copy the key so that lua_tostring does not modify the original
		lua_pushvalue( L, -2 );

		// stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
		const char *key = lua_tostring( L, -1 );

		switch( lua_type( L, -2 ) )
		{
			case LUA_TBOOLEAN:
				{
					ReadOpts.insert( QString::fromLatin1( key ), lua_toboolean( L, -2 ) );
				}
				break;

			case LUA_TNUMBER:
				{
					ReadOpts.insert( QString::fromLatin1( key ), double( lua_tonumber( L, -2 ) ) );
				}
				break;

			case LUA_TSTRING:
				{
					ReadOpts.insert( QString::fromLatin1( key ), QString::fromLatin1( lua_tostring( L, -2 ) ) );
				}
				break;
		}

		// pop value + copy of key, leaving original key
		lua_pop( L, 2 );

		// stack now contains: -1 => key; -2 => table
	}

	// stack now contains: -1 => table
	lua_pop( L, 1 );

	return( ReadOpts );
}

int lua_moo::luaRead( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	QVariantMap			 ReadOpts;
	QVariantList		 VerbArgs;

	try
	{
		const Task			&T = Command->task();
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( !lua_gettop( L ) )
		{
			InputSinkRead		*IS = new InputSinkRead( C, T, ReadOpts, VerbArgs );

			if( IS )
			{
				C->pushInputSink( IS );
			}
		}
		else if( lua_gettop( L ) == 1 )
		{
			ReadOpts = parseReadArgs( L, 1 );

			InputSinkRead		*IS = new InputSinkRead( C, T, ReadOpts, VerbArgs );

			if( IS )
			{
				C->pushInputSink( IS );
			}
		}
		else
		{
			Object				*O = lua_object::argObj( L, 1 );
			const char			*V = luaL_checkstring( L, 2 );

			if( lua_gettop( L ) > 2 )
			{
				ReadOpts = parseReadArgs( L, 3 );
			}

			for( int i = 4 ; i <= lua_gettop( L ) ; i++ )
			{
				lua_pushvalue( L, i );

				switch( lua_type( L, -1 ) )
				{
					case LUA_TBOOLEAN:
						{
							VerbArgs.append( lua_toboolean( L, -1 ) );
						}
						break;

					case LUA_TFUNCTION:
						{
							throw mooException( E_INVARG, "Can't pass a function" );
						}
						break;

					case LUA_TLIGHTUSERDATA:
						{
							throw mooException( E_INVARG, "Can't pass light userdata" );
						}
						break;

					case LUA_TUSERDATA:
						{
							lua_object::luaHandle	*H = (lua_object::luaHandle *)luaL_testudata( L, -1, lua_object::luaHandle::mLuaName );

							if( H )
							{
								QVariant		V;

								V.setValue( *H );

								VerbArgs.append( V );
							}
							else
							{
								throw mooException( E_INVARG, "Unknown userdata type" );
							}
						}
						break;

					case LUA_TNIL:
						VerbArgs.append( QVariant() );
						break;

					case LUA_TNUMBER:
						{
							VerbArgs.append( double( lua_tonumber( L, -1 ) ) );
						}
						break;

					case LUA_TSTRING:
						{
							size_t		 StrLen;
							const char	*StrDat = lua_tolstring( L, -1, &StrLen );

							if( StrDat )
							{
								VerbArgs.append( QString::fromLatin1( StrDat, StrLen ) );
							}
						}
						break;

					case LUA_TTABLE:
						{
							QVariantMap		V;

							VerbArgs.append( V );
						}
						break;
				}

				lua_pop( L, 1 );
			}

			InputSinkRead	*IS = new InputSinkRead( C, T, O->id(), V, ReadOpts, VerbArgs );

			if( IS )
			{
				C->pushInputSink( IS );
			}
		}
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

int lua_moo::luaSetCookie( lua_State *L )
{
	const char				*S = luaL_checkstring( L, 1 );

	luaL_checkany( L, 2 );

	lua_task			*Command = lua_task::luaGetTask( L );
	Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );
	QVariant			 V;

	switch( lua_type( L, 2 ) )
	{
		case LUA_TBOOLEAN:
			V = lua_toboolean( L, 2 );
			break;

		case LUA_TNUMBER:
			V = lua_tonumber( L, 2 );
			break;

		case LUA_TSTRING:
			V = QString::fromLatin1( lua_tostring( L, 2 ) );
			break;

		default:
			break;
	}

	Command->changeAdd( new change::ConnectionSetCookie( C, QString::fromLatin1( S ), V ) );

	return( 0 );
}

int lua_moo::luaCookie( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	const char				*S = luaL_checkstring( L, 1 );

	try
	{
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		QString				 N = QString::fromLatin1( S );
		QVariant			 V = C->cookie( N );

		if( V.isValid() )
		{
			luaL_pushvariant( L, V );

			return( 1 );
		}
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

int lua_moo::luaClearCookie( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	const char				*S = luaL_checkstring( L, 1 );

	try
	{
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( C->hasCookie( QString::fromLatin1( S ) ) )
		{
			Command->changeAdd( new change::ConnectionClearCookie( C, QString::fromLatin1( S ) ) );
		}
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
