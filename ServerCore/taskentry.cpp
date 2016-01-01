#include "taskentry.h"
#include "connection.h"
#include <QDateTime>
#include <QDataStream>

TaskEntry::TaskEntry( void )
{
	mConnectionId	= 0;
	mId				= 0;
	mPlayerId		= OBJECT_NONE;
	mTimeStamp		= 0;
}

TaskEntry::TaskEntry( const QString &pCommand, ConnectionId pConnectionId, ObjectId pPlayerId ):
	mCommand( pCommand )
{
	static TaskId	TID = 0;

	mId				= ++TID;
	mTimeStamp		= QDateTime::currentMSecsSinceEpoch();
	mConnectionId	= pConnectionId;
	mPlayerId		= pPlayerId;
}

void TaskEntry::save( QDataStream &pData ) const
{
	pData << mId;
	pData << mTimeStamp;
	pData << mCommand;
	pData << mPlayerId;
	pData << mConnectionId;
}

void TaskEntry::load( QDataStream &pData )
{
	pData >> mId;
	pData >> mTimeStamp;
	pData >> mCommand;
	pData >> mPlayerId;
	pData >> mConnectionId;
}
