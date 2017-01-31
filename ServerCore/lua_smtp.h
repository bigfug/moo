#ifndef LUA_SMTP_H
#define LUA_SMTP_H

#include <QThread>
#include <QSharedPointer>

#include <lua.hpp>

#include "lua_utilities.h"

#include <mimemessage.h>

class SmtpWorkerThread : public QThread
{
	Q_OBJECT

public:
	SmtpWorkerThread( QSharedPointer<MimeMessage> pMimeMessage )
		: mMimeMessage( pMimeMessage )
	{

	}

private:

	void run( void ) Q_DECL_OVERRIDE;

signals:
	void resultReady( const QString &s );

private:
	QSharedPointer<MimeMessage>		 mMimeMessage;
};

class lua_smtp
{
public:
	typedef struct luaMimeMessage
	{
		QSharedPointer<MimeMessage>		 mMimeMessage;

		static const char				*mLuaName;

	} luaMimeMessage;

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaNew( lua_State *L );

	static int luaSetSender( lua_State *L );

	static int luaAddTo( lua_State *L );

	static int luaSetSubject( lua_State *L );

	static int luaAddText( lua_State *L );

	static int luaSend( lua_State *L );

	static luaMimeMessage *message( lua_State *L, int pIndex = 1 );

private:
	static const luaL_Reg		 mLuaStatic[];

	static LuaMap				 mLuaMimeMessageMap;

	static const luaL_Reg		 mLuaMimeMessageInstance[];
	static const luaL_Reg		 mLuaMimeMessageInstanceFunctions[];

	friend class lua_moo;
};

#endif // LUA_SMTP_H
