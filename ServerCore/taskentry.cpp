#include "taskentry.h"
#include "connection.h"
#include <QDateTime>
#include <QDataStream>
#include <QDateTime>

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

bool TaskEntry::matchScheduleRange( int pValue, const QString &pRange )
{
	if( !pRange.trimmed().compare( "*" ) )
	{
		return( true );
	}

	QStringList		CommaEntries = pRange.split( ',', QString::SkipEmptyParts );

	for( QString CE : CommaEntries )
	{
		CE = CE.trimmed();

		if( CE.contains( '-' ) )
		{
			QStringList		RangeEntries = CE.split( '-' );

			if( RangeEntries.size() == 2 )
			{
				int			RangeStart = RangeEntries.at( 0 ).trimmed().toInt();
				int			RangeEnd   = RangeEntries.at( 1 ).trimmed().toInt();

				if( pValue >= RangeStart && pValue <= RangeEnd )
				{
					return( true );
				}
			}
		}
		else if( CE.toInt() == pValue )
		{
			return( true );
		}
	}

	return( false );
}

//-----------------------------------------------------------------------------
// updateTimestampFromSchedule - update timestamp with the next schedule

void TaskEntry::updateTimestampFromSchedule( qint64 pTimeStamp )
{
	if( mData.mSchedule.mYear.isEmpty() )
	{
		return;
	}

	QDateTime					 DT = QDateTime::fromMSecsSinceEpoch( pTimeStamp );
	const TaskEntrySchedule		&TS = mData.mSchedule;

	DT = DT.addSecs( 60 );

	DT.setTime( QTime( DT.time().hour(), DT.time().minute() ) );

	while( true )
	{
		if( !matchScheduleRange( DT.date().year(), TS.mYear ) )
		{
			DT = DT.addYears( 1 );

			DT.setDate( QDate( DT.date().year(), 1, 1 ) );

			DT.setTime( QTime( 0, 0 ) );

			continue;
		}

		if( !matchScheduleRange( DT.date().month(), TS.mMonth ) )
		{
			DT = DT.addMonths( 1 );

			DT.setDate( QDate( DT.date().year(), DT.date().month(), 1 ) );

			DT.setTime( QTime( 0, 0 ) );

			continue;
		}

		if( !matchScheduleRange( DT.date().day(), TS.mDayOfMonth ) )
		{
			DT = DT.addDays( 1 );

			DT.setTime( QTime( 0, 0 ) );

			continue;
		}

		if( !matchScheduleRange( DT.date().dayOfWeek(), TS.mDayOfWeek ) )
		{
			DT = DT.addDays( 1 );

			DT.setTime( QTime( 0, 0 ) );

			continue;
		}

		if( !matchScheduleRange( DT.time().hour(), TS.mHour ) )
		{
			DT = DT.addSecs( 60 * 60 );

			DT.setTime( QTime( DT.time().hour(), 0 ) );

			continue;
		}

		if( !matchScheduleRange( DT.time().minute(), TS.mMinute ) )
		{
			DT = DT.addSecs( 60 );

			continue;
		}

		break;
	}

	mData.mTimeStamp = DT.toMSecsSinceEpoch();
}


