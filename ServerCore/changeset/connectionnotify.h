#ifndef CONNECTIONNOTIFY_H
#define CONNECTIONNOTIFY_H

#include "change.h"

#include "../connection.h"

namespace change {

class ConnectionNotify : public Change
{
public:
	ConnectionNotify( Connection *pConnection, QString pMessage )
		: mConnection( pConnection ), mMessage( pMessage )
	{

	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		mConnection->notify( mMessage );
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		// do nothing
	}

private:
	Connection		*mConnection;
	QString			 mMessage;
};

}

#endif // CONNECTIONNOTIFY_H
