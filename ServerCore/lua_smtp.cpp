#include "lua_smtp.h"

#include <QSettings>

#include "lua_moo.h"
#include "lua_task.h"
#include "mooexception.h"
#include "objectmanager.h"

#include "smtpclient.h"
#include "mimetext.h"

const char	*lua_smtp::luaMimeMessage::mLuaName = "moo.smtp.message";

LuaMap		 lua_smtp::mLuaMimeMessageMap;

const luaL_Reg lua_smtp::mLuaStatic[] =
{
	{ "email", lua_smtp::luaNew },
	{ 0, 0 }
};

const luaL_Reg lua_smtp::mLuaMimeMessageInstance[] =
{
	{ "setSender", lua_smtp::luaSetSender },
	{ "set_sender", lua_smtp::luaSetSender },
	{ "addText", lua_smtp::luaAddText },
	{ "addTo", lua_smtp::luaAddTo },
	{ "add_text", lua_smtp::luaAddText },
	{ "add_to", lua_smtp::luaAddTo },
	{ "setSubject", lua_smtp::luaSetSubject },
	{ "set_subject", lua_smtp::luaSetSubject },
	{ "send", lua_smtp::luaSend },
	{ 0, 0 }
};

const luaL_Reg lua_smtp::mLuaMimeMessageInstanceFunctions[] =
{
	{ 0, 0 }
};

void lua_smtp::initialise()
{
	lua_moo::addFunctions( mLuaStatic );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaMimeMessageInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMimeMessageMap[ FP->name ] = FP->func;
	}
}

void lua_smtp::luaRegisterState( lua_State *L )
{
	luaL_newmetatable( L, luaMimeMessage::mLuaName );

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaMimeMessageInstance, 0 );

	lua_pop( L, 1 );
}

int lua_smtp::luaNew( lua_State *L )
{
	luaMimeMessage			*UD = (luaMimeMessage *)lua_newuserdata( L, sizeof( luaMimeMessage ) );

	if( !UD )
	{
		throw( mooException( E_MEMORY, "out of memory" ) );
	}

	new( &UD->mMimeMessage ) QSharedPointer<MimeMessage>( new MimeMessage() );

	luaL_getmetatable( L, luaMimeMessage::mLuaName );
	lua_setmetatable( L, -2 );

	return( 1 );
}

int lua_smtp::luaSetSender(lua_State *L)
{
	bool		LuaErr = false;

	try
	{
		luaMimeMessage		*UD   = message( L );
		size_t				 TLen;
		const char			*Text = luaL_checklstring( L, 2, &TLen );
		EmailAddress		*Addr = new EmailAddress( QString::fromLatin1( Text, TLen ) );

		if( lua_gettop( L ) >= 3 )
		{
			size_t				 NLen;
			const char			*Name = luaL_checklstring( L, 3, &NLen );

			Addr->setName( QString::fromLatin1( Name, NLen ) );
		}

		UD->mMimeMessage->setSender( Addr );
	}
	catch( mooException &e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_smtp::luaAddTo( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaMimeMessage		*UD   = message( L );
		size_t				 TLen;
		const char			*Text = luaL_checklstring( L, 2, &TLen );
		EmailAddress		*Addr = new EmailAddress( QString::fromLatin1( Text, TLen ) );

		if( lua_gettop( L ) >= 3 )
		{
			size_t				 NLen;
			const char			*Name = luaL_checklstring( L, 3, &NLen );

			Addr->setName( QString::fromLatin1( Name, NLen ) );
		}

		UD->mMimeMessage->addTo( Addr );
	}
	catch( mooException &e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_smtp::luaSetSubject( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaMimeMessage		*UD   = message( L );
		size_t				 TLen;
		const char			*Text = luaL_checklstring( L, 2, &TLen );

		UD->mMimeMessage->setSubject( QString::fromLatin1( Text, TLen ) );
	}
	catch( mooException &e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_smtp::luaAddText( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		luaMimeMessage		*UD   = message( L );
		size_t				 TLen;
		const char			*Text = luaL_checklstring( L, 2, &TLen );

		MimeText			*Mime = new MimeText( QString::fromLatin1( Text, TLen ) );

		UD->mMimeMessage->addPart( Mime );

		Mime->setParent( UD->mMimeMessage.data() );
	}
	catch( mooException &e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_smtp::luaSend( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*PRG = ObjectManager::o( T.programmer() );

		if( !PRG || !PRG->wizard() )
		{
			throw mooException( E_PERM, "only wizards can send email" );
		}

		luaMimeMessage		*UD = message( L );

		SmtpWorkerThread	*Worker = new SmtpWorkerThread( UD->mMimeMessage );

		QObject::connect( Worker, &SmtpWorkerThread::finished, Worker, &QObject::deleteLater );

		Worker->start();
	}
	catch( mooException &e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

lua_smtp::luaMimeMessage *lua_smtp::message( lua_State *L, int pIndex )
{
	luaMimeMessage *H = (luaMimeMessage *)luaL_testudata( L, pIndex, luaMimeMessage::mLuaName );

	if( !H )
	{
		throw( mooException( E_TYPE, QString( "'MimeMessage' expected for argument %1" ).arg( pIndex ) ) );
	}

	return( H );
}

void SmtpWorkerThread::run( void )
{
	QSettings		Settings( MOO_SETTINGS );

	Settings.beginGroup( "smtp" );

	QString				Host = Settings.value( "host" ).toString();
	int					Port = Settings.value( "port" ).toInt();

	QString				User = Settings.value( "user" ).toString();
	QString				Pass = Settings.value( "pass" ).toString();

	QString				Conn = Settings.value( "type" ).toString();

	Settings.endGroup();

	SmtpClient::ConnectionType		Type = SmtpClient::TcpConnection;

	if( Conn == "ssl" )
	{
		Type = SmtpClient::SslConnection;
	}

	if( Conn == "tls" )
	{
		Type = SmtpClient::TlsConnection;
	}

	SmtpClient smtp( Host, Port, Type );

	smtp.setUser( User );
	smtp.setPassword( Pass );

	smtp.connectToHost();
	smtp.login();
	smtp.sendMail( *mMimeMessage );
	smtp.quit();
}
