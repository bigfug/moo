#include "connection.h"
#include "task.h"
#include "lua_moo.h"
#include "lua_object.h"

#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QXmlInputSource>
#include <QSettings>

Connection::Connection( ConnectionId pConnectionId, QObject *pParent ) :
	QObject( pParent ), mConnectionId( pConnectionId ), mObjectId( 0 ), mPlayerId( OBJECT_NONE ), mConnectionTime( 0 ), mLastActiveTime( 0 ),
	mLineModeSupport( true ), mLastCreatedObjectId( OBJECT_NONE ), mTerminalSize( 80, 24 ), mLineMode( EDIT )
{
	mConnectionTime = mLastActiveTime = QDateTime::currentMSecsSinceEpoch();
}

bool Connection::processInput( const QString &pData )
{
	if( mLineMode == EDIT )
	{
		mLineBuffer << pData;

		while( mLineBuffer.size() > mTerminalSize.height() )
		{
			mLineBuffer.removeFirst();
		}
	}

	if( mInputSinkList.isEmpty() )
	{
		return( false );
	}

	InputSink		*IS = mInputSinkList.first();

	if( !IS )
	{
		return( false );
	}

	if( !IS->input( pData ) )
	{
		mInputSinkList.removeAll( IS );

		delete( IS );
	}

	return( true );
}

bool Connection::supportsLineMode() const
{
	return( mLineModeSupport );
}

QVariant Connection::cookie( const QString &pName )
{
	return( mCookies.value( pName ) );
}

bool Connection::hasCookie(const QString &pName) const
{
	return( mCookies.contains( pName ) );
}

void Connection::write( const QString &pText )
{
	emit textOutput( pText );
}

void Connection::notify( const QString &pText )
{
	if( mLineMode == EDIT )
	{
		mLineBuffer << pText;

		while( mLineBuffer.size() > mTerminalSize.height() )
		{
			mLineBuffer.removeFirst();
		}

		emit textOutput( pText );
	}
	else
	{
		emit textOutput( pText );
	}
}

void Connection::dataInput( const QString &pText )
{
	// Create a task entry for this data

	TaskEntry		T( pText, mConnectionId, mPlayerId );

	mLastActiveTime = T.timestamp();

	emit taskOutput( T );
}

void Connection::close()
{
	emit connectionClosed();
}

void Connection::flush()
{
	emit connectionFlush();
}

void Connection::redrawBuffer()
{
	emit textOutput( "\x1b[2J\x1b[1;1H" );

	for( const QString &S : mLineBuffer )
	{
		emit textOutput( S );
	}
}

void Connection::setLineModeSupport( bool pLineModeSupport )
{
	mLineModeSupport = pLineModeSupport;
}

void Connection::setLineMode( Connection::LineMode pLineMode )
{
	mLineMode = pLineMode;

	if( mLineModeSupport )
	{
		emit lineModeChanged( pLineMode );
	}
}

void Connection::setCookie(const QString &pName, QVariant pValue)
{
	mCookies.insert( pName, pValue );
}

void Connection::clearCookie(const QString &pName)
{
	mCookies.remove( pName );
}

void Connection::sendGMCP( const QByteArray &pGMCP )
{
	emit gmcpOutput( pGMCP );
}

