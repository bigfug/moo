#include "listenerserver.h"

ListenerServer::ListenerServer( ObjectId pObjectId, QObject *pParent ) :
	QObject( pParent ), mObjectId( pObjectId )
{
}
