#ifndef CONNECTIONSETCOOKIE_H
#define CONNECTIONSETCOOKIE_H

#include "change.h"

#include "../connection.h"

namespace change {

class ConnectionSetCookie : public Change
{
public:
	ConnectionSetCookie( Connection *pConnection, QString pCookie, QVariant pVariant )
		: mConnection( pConnection ), mCookie( pCookie )
	{
		mHadCookie = mConnection->hasCookie( mCookie );

		mOldValue  = mConnection->cookie( mCookie );

		mConnection->setCookie( mCookie, pVariant );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		if( !mHadCookie )
		{
			mConnection->clearCookie( mCookie );
		}
		else
		{
			mConnection->setCookie( mCookie, mOldValue );
		}
	}

private:
	Connection		*mConnection;
	QString			 mCookie;
	QVariant		 mOldValue;
	bool			 mHadCookie;
};

}

#endif // CONNECTIONSETCOOKIE_H
