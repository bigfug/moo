#include "lua_serialport.h"

#include <QSerialPortInfo>

#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"

const char					*lua_serialport::mLuaName = "moo.serial";
LuaMap						 lua_serialport::mLuaMap;
QMap<int,QSerialPort *>		 lua_serialport::mSerialPortMap;

const luaL_Reg lua_serialport::mLuaStatic[] =
{
	{ "openSerialPort", lua_serialport::luaOpenSerialPort },
	{ "serialPort", lua_serialport::luaPort },
	{ 0, 0 }
};

const luaL_Reg lua_serialport::mLuaInstance[] =
{
//	{ "__gc", lua_serialport::luaGC },
	{ "__index", lua_serialport::luaGet },
	{ 0, 0 }
};

const luaL_Reg lua_serialport::mLuaInstanceFunctions[] =
{
	{ "setBaudRate", lua_serialport::luaSetBaudRate },
	{ "open", lua_serialport::luaOpen },
	{ "close", lua_serialport::luaClose },
	{ "read", lua_serialport::luaRead },
	{ "write", lua_serialport::luaWrite },
	{ "register", lua_serialport::luaRegister },
	{ 0, 0 }
};

void lua_serialport::initialise()
{
	for( QSerialPortInfo SPI : QSerialPortInfo::availablePorts() )
	{
		qDebug() << SPI.portName();
	}

	lua_moo::addFunctions( mLuaStatic );
//	lua_moo::addGet( mLuaGet );
//	lua_moo::addSet( mLuaSet );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}
}

void lua_serialport::luaRegisterState( lua_State *L )
{
	// Create the moo.connection metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_serialport::mLuaInstance, 0 );

	lua_pop( L, 1 );
}

void lua_serialport::lua_pushserialport( lua_State *L, int pSerialId )
{
	luaSerialPort			*UD = (luaSerialPort *)lua_newuserdata( L, sizeof( luaSerialPort ) );

	if( !UD )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	UD->mSerialId = pSerialId;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

int lua_serialport::luaGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaSerialPort		*UD = arg( L );

		if( !mSerialPortMap.contains( UD->mSerialId ) )
		{
			return( 0 );
		}

		QSerialPort			*SP = mSerialPortMap.value( UD->mSerialId );

		const char			*s = luaL_checkstring( L, 2 );

		if( !SP )
		{
			throw( mooException( E_TYPE, "invalid serial port" ) );
		}

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		if( strcmp( s, "id" ) == 0 )
		{
			lua_pushinteger( L, UD->mSerialId );

			return( 1 );
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
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

//-----------------------------------------------------------------------------

lua_serialport::luaSerialPort * lua_serialport::arg( lua_State *L, int pIndex )
{
	luaSerialPort *H = (luaSerialPort *)luaL_testudata( L, pIndex, mLuaName );

	if( H == 0 )
	{
		throw( mooException( E_TYPE, QString( "'serialport' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

int lua_serialport::luaOpenSerialPort( lua_State *L )
{
	const char		*s = luaL_checkstring( L, 1 );
	QString			 P = QString::fromLatin1( s );

	for( QMap<int,QSerialPort *>::iterator it = mSerialPortMap.begin() ; it != mSerialPortMap.end() ; it++ )
	{
		if( it.value()->portName() == P )
		{
			lua_pushserialport( L, it.key() );

			return( 1 );
		}
	}

	static int	SPID = 0;

	mSerialPortMap.insert( ++SPID, new QSerialPort( P ) );

	QSerialPort		&SP = *mSerialPortMap.value( SPID );

	SP.setProperty( "sid", SPID );

	lua_pushserialport( L, SPID );

	return( 1 );
}

int lua_serialport::luaPort( lua_State *L )
{
	int				 ID = luaL_checkinteger( L, 1 );

	if( mSerialPortMap.contains( ID ) )
	{
		lua_pushserialport( L, ID );

		return( 1 );
	}

	return( 0 );
}

int lua_serialport::luaSetBaudRate(lua_State *L)
{
	luaSerialPort		*UD = arg( L );

	if( !mSerialPortMap.contains( UD->mSerialId ) )
	{
		return( 0 );
	}

	QSerialPort		&SP = *mSerialPortMap.value( UD->mSerialId );

	int					 BaudRate = luaL_checkinteger( L, 2 );

	SP.setBaudRate( BaudRate );

	return( 0 );
}

int lua_serialport::luaGC( lua_State *L )
{
	luaSerialPort		*UD = arg( L );

	QSerialPort			*SP = mSerialPortMap.value( UD->mSerialId );

	mSerialPortMap.remove( UD->mSerialId );

	SP->deleteLater();

	return( 0 );
}

int lua_serialport::luaOpen( lua_State *L )
{
	luaSerialPort		*UD = arg( L );

	if( !mSerialPortMap.contains( UD->mSerialId ) )
	{
		return( 0 );
	}

	QSerialPort		&SP = *mSerialPortMap.value( UD->mSerialId );

	if( SP.isOpen() )
	{
		lua_pushboolean( L, true );

		return( 1 );
	}

	lua_pushboolean( L, SP.open( QSerialPort::ReadWrite ) );

	if( SP.error() != QSerialPort::NoError )
	{
		luaL_error( L, SP.errorString().toLatin1() );
	}

	return( 1 );
}

int lua_serialport::luaClose(lua_State *L)
{
	luaSerialPort		*UD = arg( L );

	if( !mSerialPortMap.contains( UD->mSerialId ) )
	{
		return( 0 );
	}

	QSerialPort		&SP = *mSerialPortMap.value( UD->mSerialId );

	SP.close();

	return( 0 );
}

int lua_serialport::luaRead(lua_State *L)
{
	luaSerialPort		*UD = arg( L );

	if( !mSerialPortMap.contains( UD->mSerialId ) )
	{
		return( 0 );
	}

	QSerialPort		&SP = *mSerialPortMap.value( UD->mSerialId );

	if( !SP.isReadable() || SP.bytesAvailable() <= 0 )
	{
		return( 0 );
	}

	QByteArray			 UB = SP.readAll();

	lua_pushlstring( L, UB.constData(), UB.size() );

	return( 1 );
}

int lua_serialport::luaWrite(lua_State *L)
{
	luaSerialPort		*UD = arg( L );

	if( !mSerialPortMap.contains( UD->mSerialId ) )
	{
		return( 0 );
	}

	QSerialPort		&SP = *mSerialPortMap.value( UD->mSerialId );

	if( !SP.isWritable() )
	{
		return( 0 );
	}

	size_t			DatLen;

	const char		*DatPtr = luaL_checklstring( L, 2, &DatLen );

	qint64			DatOut = 0;

	if( DatLen > 0 )
	{
		DatOut = SP.write( DatPtr, DatLen );
	}

	lua_pushinteger( L, DatOut );

	return( 1 );
}

int lua_serialport::luaRegister(lua_State *L)
{
	luaSerialPort		*UD = arg( L );
	Object				*O  = lua_object::argObj( L, 2 );

	QSerialPort			*SP = mSerialPortMap.value( UD->mSerialId );

	SP->setProperty( "oid", O->id() );

	QObject::connect( SP, &QSerialPort::readyRead, [=]()
	{
		int				ObjectId = SP->property( "oid" ).toInt();
		int				SerialId = SP->property( "sid" ).toInt();

		TaskEntry		TE( QString( "o( %1 ):serialReadReady( %2 )" ).arg( ObjectId ).arg( SerialId ), 0, ObjectId );

		ObjectManager::instance()->doTask( TE );
	} );

	return( 0 );
}
