#ifndef LISTENER_H
#define LISTENER_H

#include "mooglobal.h"

#include <QObject>

class ListenerServer : public QObject
{
	Q_OBJECT

public:
	explicit ListenerServer( ObjectId pObjectId, QObject *pParent = 0 );

	virtual ~ListenerServer( void ) {}

	inline ObjectId objectid( void ) const
	{
		return( mObjectId );
	}

protected:
	ObjectId					 mObjectId;
};

#endif // LISTENER_H
