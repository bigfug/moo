#ifndef LISTENER_H
#define LISTENER_H

#include "mooglobal.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

class ListenerSocket : public QObject
{
	Q_OBJECT

public:
	explicit ListenerSocket( QObject *pParent, QTcpSocket *pSocket );

	inline ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

private:
	void sendData( const QByteArray &pData );
	void processInput( const QByteArray &pData );
	void processTelnetSequence( const QByteArray &pData );
	void processAnsiSequence( const QByteArray &pData );

private slots:
	void disconnected( void );
	void readyRead( void );
	void textInput( const QString &pText );
	void inputTimeout( void );

signals:
	void textOutput( const QString &pText );

private:
	ConnectionId	 mConnectionId;
	QTcpSocket		*mSocket;
	QString			 mBuffer;
	uint8_t			 mLastChar;
	QByteArray		 mTelnetSequence;
	int				 mTelnetDepth;
	bool			 mDataReceived;
	bool			 mLocalEcho;
	bool			 mWebSocketActive;
	bool			 mWebSocketHeader;
	QString			 mWebSocketAccept;
	QByteArray		 mWebSocketBuffer;
	QTimer			 mTimer;
	int				 mAnsiEsc;
	QByteArray		 mAnsiSeq;
	int				 mAnsiPos;
};

class Listener : public QObject
{
    Q_OBJECT

public:
	explicit Listener( ObjectId pObjectId, quint16 pPort, QObject *pParent = 0 );

	virtual ~Listener( void );

	inline ObjectId objectid( void ) const
	{
		return( mObjectId );
	}

private slots:
	void newConnection( void );

private:
	ObjectId				mObjectId;
	QTcpServer				mServer;
	ObjectId				mListeningObjectId;
};

#endif // LISTENER_H
