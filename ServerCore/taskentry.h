#ifndef TASKENTRY_H
#define TASKENTRY_H

#include <QString>

#include "mooglobal.h"

class QDataStream;

typedef struct TaskEntryData
{
	static TaskId		 TID;

	TaskId				 mId;					// the task id
	qint64				 mTimeStamp;			// when the task was created
	QString				 mCommand;				// the command
	ObjectId			 mPlayerId;				// the player who typed the command
	ConnectionId		 mConnectionId;
} TaskEntryData;

class TaskEntry
{
	friend class ODB;

public:
	TaskEntry( void );

	TaskEntry( const QString &pCommand, ConnectionId pConnectionId, ObjectId pPlayerId = OBJECT_NONE );

	inline TaskId id( void ) const
	{
		return( mData.mId );
	}

	inline qint64 timestamp( void ) const
	{
		return( mData.mTimeStamp );
	}

	inline const QString &command( void ) const
	{
		return( mData.mCommand );
	}

	inline ObjectId playerid( void ) const
	{
		return( mData.mPlayerId );
	}

	inline ConnectionId connectionid( void ) const
	{
		return( mData.mConnectionId );
	}

	inline void setTimeStamp( qint64 pTimeStamp )
	{
		mData.mTimeStamp = pTimeStamp;
	}

	static bool lessThan( const TaskEntry &s1, const TaskEntry &s2 )
	{
		return s1.mData.mTimeStamp < s2.mData.mTimeStamp;
	}

protected:
	TaskEntryData &data( void )
	{
		return( mData );
	}

	const TaskEntryData &data( void ) const
	{
		return( mData );
	}

private:
	TaskEntryData		mData;
};

#endif // TASKENTRY_H
