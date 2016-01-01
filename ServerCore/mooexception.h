#ifndef MOOEXCEPTION_H
#define MOOEXCEPTION_H

#include <lua.hpp>

#include "mooglobal.h"
#include <QDebug>

class mooException
{
public:
	mooException( mooError pError, const QString &pMessage ) : mError( pError ), mMessage( pMessage )
	{
	}

	mooException( mooError pError, const char *pMessage ) : mError( pError ), mMessage( pMessage )
	{
	}

	void lua_pushexception( lua_State *L )
	{
		qDebug() << mMessage;

		luaL_where( L, 1 );
		lua_pushstring( L, mMessage.toLatin1() );
		lua_concat( L, 2 );
	}

public:
	const mooError	mError;
	const QString	mMessage;
};

#endif // MOOEXCEPTION_H
