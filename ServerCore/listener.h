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

typedef struct TelnetOption
{
	quint8			mOption;
	quint8			mLocal;
	quint8			mRemote;

	TelnetOption( quint8 pOption, quint8 pLocal = 0, quint8 pRemote = 0 )
		: mOption( pOption ), mLocal( pLocal ), mRemote( pRemote )
	{

	}
} TelnetOption;

class ListenerSocket : public QObject
{
	Q_OBJECT

public:
	explicit ListenerSocket( QObject *pParent, QTcpSocket *pSocket );

	inline ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

	void setOptions( const QList<TelnetOption> &pOptions )
	{
		mOptions = pOptions;
	}

	bool echo( void ) const;

	bool option( quint8 pOption ) const;

private:
	void sendData( const QByteArray &pData );
	void processInput( const QByteArray &pData );
	void processTelnetSequence( const QByteArray &pData );
	void processAnsiSequence( const QByteArray &pData );

	static void appendTelnetSequence( QByteArray &pA, const quint8 p1, const quint8 p2 );

private slots:
	void disconnected( void );
	void readyRead( void );
	void textInput( const QString &pText );
	void inputTimeout( void );

	void setLineMode( Connection::LineMode pLineMode );

	void setTelnetOption( quint8 pOption, quint8 pCommand );

signals:
	void textOutput( const QString &pText );
	void lineModeSupported( bool pLineModeSupport );

private:
	ConnectionId			 mConnectionId;
	QTcpSocket				*mSocket;
	QString					 mBuffer;
	quint8					 mLastChar;
	QByteArray				 mTelnetSequence;
	bool					 mDataReceived;
	bool					 mWebSocketActive;
	bool					 mWebSocketHeader;
	QString					 mWebSocketAccept;
	QByteArray				 mWebSocketBuffer;
	QTimer					 mTimer;
	int						 mAnsiEsc;
	QByteArray				 mAnsiSeq;
	int						 mAnsiPos;
	QList<TelnetOption>		 mOptions;
	Connection::LineMode	 mLineMode;
	bool					 mTelnetOptionsSent;
	bool					 mTelnetOptionsReceived;
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
	ObjectId				 mObjectId;
	QTcpServer				 mServer;
	ObjectId				 mListeningObjectId;
	QList<TelnetOption>		 mOptions;
};

#endif // LISTENER_H
