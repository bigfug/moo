#ifndef TASKENTRY_H
#define TASKENTRY_H

#include <QString>

#include "mooglobal.h"

class QDataStream;

class TaskEntry
{
public:
	TaskEntry( void );

	TaskEntry( const QString &pCommand, ConnectionId pConnectionId, ObjectId pPlayerId = OBJECT_NONE );

	void save( QDataStream &pData ) const;
	void load( QDataStream &pData );

	inline TaskId id( void ) const
	{
		return( mId );
	}

	inline qint64 timestamp( void ) const
	{
		return( mTimeStamp );
	}

	inline const QString &command( void ) const
	{
		return( mCommand );
	}

	inline ObjectId playerid( void ) const
	{
		return( mPlayerId );
	}

	inline ConnectionId connectionid( void ) const
	{
		return( mConnectionId );
	}

	inline void setTimeStamp( qint64 pTimeStamp )
	{
		mTimeStamp = pTimeStamp;
	}

	static bool lessThan( const TaskEntry &s1, const TaskEntry &s2 )
	{
		return s1.mTimeStamp < s2.mTimeStamp;
	}

private:
	TaskId				 mId;					// the task id
	qint64				 mTimeStamp;			// when the task was created
	QString				 mCommand;				// the command
	ObjectId			 mPlayerId;				// the player who typed the command
	ConnectionId		 mConnectionId;
};

#endif // TASKENTRY_H
