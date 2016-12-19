#include "taskentry.h"
#include "connection.h"
#include <QDateTime>
#include <QDataStream>

TaskId	TaskEntryData::TID = 0;

TaskEntry::TaskEntry( void )
{
	mData.mConnectionId	= 0;
	mData.mId			= 0;
	mData.mPlayerId		= OBJECT_NONE;
	mData.mTimeStamp	= 0;
}

TaskEntry::TaskEntry( const QString &pCommand, ConnectionId pConnectionId, ObjectId pPlayerId )
{
	mData.mId			= ++TaskEntryData::TID;
	mData.mTimeStamp	= QDateTime::currentMSecsSinceEpoch();
	mData.mCommand		= pCommand;
	mData.mConnectionId	= pConnectionId;
	mData.mPlayerId		= pPlayerId;
}
