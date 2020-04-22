#ifndef TASKENTRY_H
#define TASKENTRY_H

#include <QString>

#include "mooglobal.h"

class QDataStream;

typedef struct TaskEntrySchedule
{
	QString				 mMinute;
	QString				 mHour;
	QString				 mDayOfWeek;
	QString				 mDayOfMonth;
	QString				 mMonth;
	QString				 mYear;
} TaskEntrySchedule;

typedef struct TaskEntryData
{
	TaskId				 mId;					// the task id
	qint64				 mTimeStamp;			// when the task was created
	QString				 mCommand;				// the command
	ObjectId			 mPlayerId;				// the player who typed the command
	ConnectionId		 mConnectionId;
	TaskEntrySchedule	 mSchedule;
} TaskEntryData;

class TaskEntry
{
	friend class ODB;

	static TaskId TID;

public:
	static TaskId newTaskId( void )
	{
		return( ++TID );
	}

	static void setMaxTaskId( TaskId pTaskId )
	{
		TID = std::max( TID, pTaskId );
	}

	TaskEntry( void );

	TaskEntry( const QString &pCommand, ConnectionId pConnectionId, ObjectId pPlayerId = OBJECT_NONE );

	void updateTimestampFromSchedule( qint64 pTimeStamp );

	TaskId id( void ) const
	{
		return( mData.mId );
	}

	qint64 timestamp( void ) const
	{
		return( mData.mTimeStamp );
	}

	const QString &command( void ) const
	{
		return( mData.mCommand );
	}

	ObjectId playerid( void ) const
	{
		return( mData.mPlayerId );
	}

	ConnectionId connectionid( void ) const
	{
		return( mData.mConnectionId );
	}

	void setTimeStamp( qint64 pTimeStamp )
	{
		mData.mTimeStamp = pTimeStamp;
	}

	static bool lessThan( const TaskEntry &s1, const TaskEntry &s2 )
	{
		return s1.mData.mTimeStamp < s2.mData.mTimeStamp;
	}

	TaskEntrySchedule schedule( void ) const
	{
		return( mData.mSchedule );
	}

	void setSchedule( const TaskEntrySchedule &pSchedule )
	{
		mData.mSchedule = pSchedule;
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

	bool matchScheduleRange( int pValue, const QString &pRange );

private:
	TaskEntryData		mData;
};

#endif // TASKENTRY_H
