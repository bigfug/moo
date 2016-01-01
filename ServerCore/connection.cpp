#include "connection.h"
#include "task.h"
#include "lua_moo.h"
#include "lua_object.h"

#include <QDebug>
#include <QDateTime>

Connection::Connection( ConnectionId pConnectionId, QObject *pParent ) :
	QObject( pParent ), mConnectionId( pConnectionId ), mObjectId( 0 ), mPlayerId( OBJECT_NONE ), mConnectionTime( 0 ), mLastActiveTime( 0 )
{
	mConnectionTime = mLastActiveTime = QDateTime::currentMSecsSinceEpoch();
}

bool Connection::processInput( const QString &pData )
{
	if( mInputSinkList.isEmpty() )
	{
		return( false );
	}

	InputSink		*IS = mInputSinkList.first();

	if( IS == 0 )
	{
		return( false );
	}

	if( !IS->input( pData ) )
	{
		mInputSinkList.removeFirst();

		delete( IS );
	}

	return( true );
}

void Connection::notify( const QString &pText )
{
	//qDebug() << pText;

	emit textOutput( pText );
}

void Connection::dataInput( const QString &pText )
{
	// Create a task entry for this data

	TaskEntry		T( pText, mConnectionId, mPlayerId );

	mLastActiveTime = T.timestamp();

	emit taskOutput( T );
}
