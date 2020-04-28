#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QSize>
#include <QMap>
#include <QVector>
#include <QVariant>
#include <QString>
#include <QXmlDefaultHandler>
#include <QRect>

#include "mooglobal.h"
#include "taskentry.h"

class InputSink;

class Connection : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY( Connection )

	explicit Connection( ConnectionId pConnectionId, QObject *pParent = Q_NULLPTR );

	void setObjectId( const ObjectId pObjectId )
	{
		mObjectId = pObjectId;
	}

	friend class ConnectionManager;

public:
	typedef enum LineMode
	{
		NOT_SET = -1,
		EDIT,
		REALTIME
	} LineMode;

	ConnectionId id( void ) const
	{
		return( mConnectionId );
	}

	QString name( void ) const
	{
		return( mName );
	}

	ObjectId player( void ) const
	{
		return( mPlayerId );
	}

	ObjectId object( void ) const
	{
		return( mObjectId );
	}

	const QString &prefix( void ) const
	{
		return( mPrefix );
	}

	const QString &suffix( void ) const
	{
		return( mSuffix );
	}

	void setPrefix( const QString &pPrefix )
	{
		mPrefix = pPrefix;
	}

	void setSuffix( const QString &pSuffix )
	{
		mSuffix = pSuffix;
	}

	void setPlayerId( ObjectId pPlayerId )
	{
		mPlayerId = pPlayerId;
	}

	void pushInputSink( InputSink *pIS );

	bool processInput( const QString &pData );

	bool supportsLineMode( void ) const;

	ObjectId lastCreatedObjectId( void ) const
	{
		return( mLastCreatedObjectId );
	}

	QSize terminalSize( void ) const
	{
		return( mTerminalSize );
	}

	LineMode lineMode( void ) const
	{
		return( mLineMode );
	}

	QVariant cookie( const QString &pName );

	bool hasCookie( const QString &pName ) const;

	QRect terminalWindow( void ) const
	{
		return( mTerminalWindow );
	}

	ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

signals:
	void taskOutput( TaskEntry &pTask );
	void listenerOutput( const QString &pText );	// data to be sent to the listener
	void lineModeChanged( Connection::LineMode pLineMode );
	void gmcpOutput( const QByteArray &pGMCP );
	void connectionClosed( void );
	void connectionFlush( void );

public slots:
	void write( const QString &pText );				// writes directly to listener
	void notify( const QString &pText );			// stores to line buffer and writes to listener if allowed

	void listenerInput( const QString &pText );		// data coming in from listener

	void close( void );
	void flush( void );

	void redrawBuffer( void );

	void setLineModeSupport( bool pLineModeSupport );
	void setLineMode( LineMode pLineMode );

	void addToLineBuffer( const QString &pText );

	void setLastCreatedObjectId( const ObjectId pObjectId )
	{
		mLastCreatedObjectId = pObjectId;
	}

	void performTask( const QString &pTask );

	void setTerminalSize( QSize pSize )
	{
		int		l = mTerminalWindow.left();
		int		r = mTerminalSize.width() - mTerminalWindow.right();
		int		t = mTerminalWindow.top();
		int		b = mTerminalSize.height() - mTerminalWindow.bottom();

		mTerminalSize = pSize;

		mTerminalWindow = QRect( QPoint( l, t ), mTerminalSize - QSize( r, b ) );
	}

	void setTerminalWindowBottom( int v )
	{
		mTerminalWindow.setBottom( v );
	}

	void setTerminalWindow( QRect pWindow )
	{
		mTerminalWindow = pWindow;
	}

	void resetTerminalWindow( void )
	{
		mTerminalWindow = QRect( QPoint(), mTerminalSize );
	}

	void setCookie( const QString &pName, QVariant pValue );

	void clearCookie( const QString &pName );

	void sendGMCP( const QByteArray &pGMCP );

private:
	ConnectionId			 mConnectionId;
	ObjectId				 mObjectId;
	ObjectId				 mPlayerId;
	QString					 mPrefix;
	QString					 mSuffix;
	qint64					 mConnectionTime;
	qint64					 mLastActiveTime;
	QString					 mName;
	QVector<InputSink *>	 mInputSinkList;
	bool					 mLineModeSupport;
	ObjectId				 mLastCreatedObjectId;
	QSize					 mTerminalSize;
	QStringList				 mLineBuffer;
	LineMode				 mLineMode;
	QMap<QString,QVariant>	 mCookies;
	QRect					 mTerminalWindow;
};

#endif // CONNECTION_H
