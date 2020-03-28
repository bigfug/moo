#ifndef CONNECTIONCLOSE_H
#define CONNECTIONCLOSE_H

#include "change.h"

#include "../connection.h"

namespace change {

class ConnectionClose : public Change
{
public:
	ConnectionClose( Connection *pConnection )
		: mConnection( pConnection )
	{

	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		mConnection->close();
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		// do nothing
	}

private:
	Connection		*mConnection;
};

}

#endif // CONNECTIONCLOSE_H
