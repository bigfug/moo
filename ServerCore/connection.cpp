#include "connection.h"

#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QXmlInputSource>
#include <QSettings>

#include "task.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "inputsink/inputsink.h"
#include "inputsink/inputsinkcommand.h"
#include "taskentry.h"
#include "task.h"

Connection::Connection( ConnectionId pConnectionId, QObject *pParent ) :
	QObject( pParent ), mConnectionId( pConnectionId ), mObjectId( 0 ), mPlayerId( OBJECT_NONE ), mConnectionTime( 0 ), mLastActiveTime( 0 ),
	mLineModeSupport( true ), mLastCreatedObjectId( OBJECT_NONE ), mTerminalSize( 80, 24 ), mLineMode( EDIT )
{
	mConnectionTime = mLastActiveTime = QDateTime::currentMSecsSinceEpoch();

	resetTerminalWindow();

	InputSinkCommand	*IS = new InputSinkCommand( this );

	pushInputSink( IS );
}

void Connection::pushInputSink( InputSink *pIS )
{
	mInputSinkList.push_front( pIS );

	setLineMode( pIS->lineMode() );
}

/*
 * called from lua_task::execute( qint64 pTimeStamp )
 * returns true if we don't want to process any further
 * this allows InputSink classes to filter input
 *
 * @param pData the command entered by the player
*/

bool Connection::processInput( const QString &pData )
{
	return( false );

//	qDebug() << "Connection::processInput" << pData;

//	if( true )
//	{
//		return( false );
//	}

//	if( mLineMode == EDIT )
//	{
//		mLineBuffer << pData;

//		while( mLineBuffer.size() > mTerminalSize.height() )
//		{
//			mLineBuffer.removeFirst();
//		}
//	}

//	if( mInputSinkList.isEmpty() )
//	{
//		return( false );
//	}

//	InputSink		*IS = mInputSinkList.first();

//	if( !IS )
//	{
//		return( false );
//	}

//	if( !IS->input( pData ) )
//	{
//		if( IS->screenNeedsReset() )
//		{
//			redrawBuffer();
//		}

//		mInputSinkList.removeAll( IS );

//		delete( IS );
//	}

//	if( !mInputSinkList.isEmpty() )
//	{
//		setLineMode( mInputSinkList.first()->lineMode() );

//		mInputSinkList.first()->output( "" );
//	}
//	else
//	{
//		setLineMode( Connection::EDIT );
//	}

//	return( true );
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
	emit listenerOutput( pText );
}

void Connection::notify( const QString &pText )
{
	bool		PrintText = true;

	if( !mInputSinkList.isEmpty() )
	{
		if( mInputSinkList.first()->output( pText ) )
		{
			PrintText = false;
		}
	}

	if( mLineMode == EDIT )
	{
		addToLineBuffer( pText );
	}

	if( PrintText )
	{
		write( pText );
		write( "\r\n" );
	}
}

void Connection::listenerInput( const QString &pText )
{
//	while( lineMode() == REALTIME )
//	{

//	}

//	for( int i = 0 ; i < pData.size() ; i++ )
//	{
//		quint8		ch = pData.at( i );

//		if( mLineMode == Connection::REALTIME )
//		{
//			emit charOutput( ch );
//		}
//		else
//		{
//			mLineEdit.dataInput( pData );
//		}
//	}

//	qDebug() << "listenerInput" << pText;

	if( !mInputSinkList.isEmpty() )
	{
		InputSink		*IS = mInputSinkList.first();

		if( IS )
		{
			if( !IS->input( pText ) )
			{
				if( IS->screenNeedsReset() )
				{
					redrawBuffer();
				}

				mInputSinkList.removeAll( IS );

				delete( IS );
			}
		}
	}
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
	write( QString( "\x1b[2J" ) );

	for( int y = mTerminalWindow.top() ; y < mTerminalWindow.bottom() ; y++ )
	{
		write( QString( "\x1b[%1;%2H" ).arg( y + 1 ).arg( mTerminalWindow.left() + 1 ) );

		if( mLineBuffer.size() <= y )
		{
			break;
		}

		write( mLineBuffer.at( y ) );
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

void Connection::addToLineBuffer( const QString &pText )
{
	int			i = 0;

	while( i < pText.length() )
	{
		int		j = std::min( pText.length(), mTerminalWindow.width() );

		mLineBuffer << pText.mid( i, j );

		i += j;
	}

	while( mLineBuffer.size() > mTerminalSize.height() )
	{
		mLineBuffer.removeFirst();
	}
}

void Connection::performTask( const QString &pTask )
{
	TaskEntry	E( pTask, connectionId(), player() );

	emit taskOutput( E );

	mLastActiveTime = E.timestamp();
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

