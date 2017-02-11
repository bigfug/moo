#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QSize>

#include "mooglobal.h"
#include "taskentry.h"
#include "inputsink.h"

class Connection : public QObject
{
	Q_OBJECT

	explicit Connection( ConnectionId pConnectionId, QObject *pParent = 0 );

	inline void setObjectId( const ObjectId pObjectId )
	{
		mObjectId = pObjectId;
	}

	friend class ConnectionManager;

public:
	typedef enum LineMode
	{
		EDIT,
		REALTIME
	} LineMode;

	inline ConnectionId id( void ) const
	{
		return( mConnectionId );
	}

	inline QString name( void ) const
	{
		return( mName );
	}

	inline ObjectId player( void ) const
	{
		return( mPlayerId );
	}

	inline ObjectId object( void ) const
	{
		return( mObjectId );
	}

	inline const QString &prefix( void ) const
	{
		return( mPrefix );
	}

	inline const QString &suffix( void ) const
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

	inline void setPlayerId( ObjectId pPlayerId )
	{
		mPlayerId = pPlayerId;
	}

	inline void pushInputSink( InputSink *pIS )
	{
		mInputSinkList.push_front( pIS );
	}

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

	inline LineMode lineMode( void ) const
	{
		return( mLineMode );
	}

signals:
	void taskOutput( TaskEntry &pTask );
	void textOutput( const QString &pText );
	void lineModeChanged( Connection::LineMode pLineMode );

public slots:
	void write( const QString &pText );
	void notify( const QString &pText );
	void dataInput( const QString &pText );

	void redrawBuffer( void );

	void setLineModeSupport( bool pLineModeSupport );
	void setLineMode( LineMode pLineMode );

	void setLastCreatedObjectId( const ObjectId pObjectId )
	{
		mLastCreatedObjectId = pObjectId;
	}

	void setTerminalSize( QSize pSize )
	{
		mTerminalSize = pSize;
	}

private:
	ConnectionId		mConnectionId;
	ObjectId			mObjectId;
	ObjectId			mPlayerId;
	QString				mPrefix;
	QString				mSuffix;
	qint64				mConnectionTime;
	qint64				mLastActiveTime;
	QString				mName;
	QList<InputSink *>	mInputSinkList;
	bool				mLineModeSupport;
	ObjectId			mLastCreatedObjectId;
	QSize				mTerminalSize;
	QStringList			mLineBuffer;
	LineMode			mLineMode;
};

#endif // CONNECTION_H
