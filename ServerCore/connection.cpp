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

QString preprocessString( const QString &S )
{
	QString		O;
	QString		E;
	bool		Escaped = false;

	for( QChar C : S )
	{
		if( Escaped )
		{
			if( E.isEmpty() )
			{
				if( C == 'a' )
				{
					O.append( '\a' ); Escaped = false; continue;
				}

				if( C == 'b' )
				{
					O.append( '\b' ); Escaped = false; continue;
				}

				if( C == 'e' )
				{
					O.append( '\e' ); Escaped = false; continue;
				}

				if( C == 'f' )
				{
					O.append( '\f' ); Escaped = false; continue;
				}

				if( C == 'n' )
				{
					O.append( '\n' ); Escaped = false; continue;
				}

				if( C == 'r' )
				{
					O.append( '\r' ); Escaped = false; continue;
				}

				if( C == 't' )
				{
					O.append( '\t' ); Escaped = false; continue;
				}

				if( C == '\'' )
				{
					O.append( '\'' ); Escaped = false; continue;
				}

				if( C == '\\' )
				{
					O.append( '\\' ); Escaped = false; continue;
				}

				if( C == '"' )
				{
					O.append( '"' ); Escaped = false; continue;
				}

				if( C == 'x' )
				{
					E = C; continue;
				}

				if( C.isDigit() )
				{
					E = C; continue;
				}

				O.append( '\\' );
				O.append( C );

				Escaped = false;

				continue;
			}

			if( E[ 0 ] == 'x' )
			{
				QChar	c = C.toLower();

				if( c >= 'a' && c <= 'f' )
				{
					E.append( c ); continue;
				}
				else
				{
					QChar	V;
					bool	ok;

					E.remove( 0, 1 );

					V = E.toUInt( &ok, 16 );

					if( ok )
					{
						O.append( V );
					}
				}
			}
			else if( E[ 0 ].isDigit() )
			{
				if( C.isDigit() )
				{
					E.append( C ); continue;
				}
				else
				{
					QChar	V;
					bool	ok;

					V = E.toUInt( &ok, 8 );

					if( ok )
					{
						O.append( V );
					}
				}
			}
		}

		if( C == '\\' )
		{
			Escaped = true;

			E.clear();
		}
		else
		{
			O.append( C );
		}
	}

	return( O );
}

bool Connection::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( qName )
	Q_UNUSED( atts )

	if( localName != "moo" )
	{
		QString			Style = QSettings( MOO_SETTINGS ).value( QString( "style/%1" ).arg( qName ) ).toString();

		//Style = preprocessString( Style );

		mXML.append( Style );

		mStyles.append( Style );
	}

	return( true );
}

bool Connection::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( localName )
	Q_UNUSED( qName )

	if( localName != "moo" )
	{
		if( !mStyles.isEmpty() )
		{
			mStyles.removeLast();
		}

		if( !mStyles.isEmpty() )
		{
			mXML.append( mStyles.last() );
		}
		else
		{
			mXML.append( "\x1b[0m" );
		}
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
