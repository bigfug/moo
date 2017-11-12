#include "connection.h"
#include "task.h"
#include "lua_moo.h"
#include "lua_object.h"

#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QXmlInputSource>

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

void Connection::write( const QString &pText )
{
	emit textOutput( pText );
}

void Connection::notify( const QString &pText )
{
	if( mLineMode == EDIT )
	{
		mXML.clear();

		QXmlInputSource		XmlSrc;

		XmlSrc.setData( "<moo>" + pText + "</moo>" );

		QXmlSimpleReader	Reader;

		Reader.setContentHandler( this );
		Reader.setEntityResolver( this );

		Reader.parse( XmlSrc );

		mLineBuffer << mXML;

		while( mLineBuffer.size() > mTerminalSize.height() )
		{
			mLineBuffer.removeFirst();
		}

//		qDebug() << mXML;

		emit textOutput( mXML );
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

bool Connection::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( qName )
	Q_UNUSED( atts )

	if( localName == "b" )
	{
		mXML.append( "\x1b[1m" );
	}
	else if( localName == "u" )
	{
		mXML.append( "\x1b[4m" );
	}
	else if( localName == "red" )
	{
		mXML.append( "\x1b[31m" );
	}

	return( true );
}

bool Connection::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( qName )

	if( localName == "b" )
	{
		mXML.append( "\x1b[0m" );
	}
	else if( localName == "u" )
	{
		mXML.append( "\x1b[0m" );
	}
	else if( localName == "red" )
	{
		mXML.append( "\x1b[0m" );
	}

	return( true );
}

QString Connection::errorString() const
{
	return( QString() );
}

bool Connection::characters(const QString &ch)
{
//	qDebug() << "characters" << ch;

	mXML.append( ch );

	return( true );
}


bool Connection::skippedEntity(const QString &name)
{
//	qDebug() << "skippedEntity" << name;

	mXML.append( name );

	return( true );
}
