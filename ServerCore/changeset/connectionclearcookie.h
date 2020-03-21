#ifndef CONNECTIONCLEARCOOKIE_H
#define CONNECTIONCLEARCOOKIE_H

#include "change.h"

#include "../connection.h"

namespace change {

class ConnectionClearCookie : public Change
{
public:
	ConnectionClearCookie( Connection *pConnection, QString pCookie )
		: mConnection( pConnection ), mCookie( pCookie )
	{
		mHadCookie = mConnection->hasCookie( mCookie );

		mOldValue = mConnection->cookie( mCookie );

		mConnection->clearCookie( mCookie );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		if( mHadCookie )
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

#endif // CONNECTIONCLEARCOOKIE_H
