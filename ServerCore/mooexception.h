#ifndef MOOEXCEPTION_H
#define MOOEXCEPTION_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <QString>

#include "mooglobal.h"

class mooException
{
public:
	mooException( void )
		: mError( E_NONE )
	{
	}

	mooException( mooError pError, const QString &pMessage ) : mError( pError ), mMessage( pMessage )
	{
	}

	mooException( mooError pError, const char *pMessage ) : mError( pError ), mMessage( pMessage )
	{
	}

	void lua_pushexception( lua_State *L ) const;

	mooError error( void ) const
	{
		return( mError );
	}

	QString message( void ) const
	{
		return( mMessage );
	}

public:
	mooError	mError;
	QString		mMessage;
};

#endif // MOOEXCEPTION_H
