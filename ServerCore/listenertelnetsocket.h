#ifndef LISTENERTELNETSOCKET_H
#define LISTENERTELNETSOCKET_H

#include "mooglobal.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

#include "libtelnet.h"

#include "listenersocket.h"

class ListenerTelnetSocket : public ListenerSocket
{
	Q_OBJECT

public:
	explicit ListenerTelnetSocket( QObject *pParent, QTcpSocket *pSocket );

	virtual ~ListenerTelnetSocket( void ) {}

	void setOptions( const QVector<telnet_telopt_t> &pOptions )
	{
		mOptions = pOptions;
	}

	bool echo( void ) const;

private:
	void sendData( const QByteArray &pData );
	void processInput( const QByteArray &pData );

	static void telnetEventHandlerStatic( telnet_t *telnet, telnet_event_t *event, void *user_data );

	void telnetEventHandler( telnet_event_t *event );

private slots:
	void disconnected( void );
	void readyRead( void );
	void textInput( const QString &pText );
	void inputTimeout( void );

	void setLineMode( Connection::LineMode pLineMode );

	void sendGMCP( const QByteArray &pGMCP );

signals:
	void textOutput( const QString &pText );
	void lineModeSupported( bool pLineModeSupport );

private:
	QTcpSocket					*mSocket;
	QString						 mBuffer;
	int							 mCursorPosition;
	quint8						 mLastChar;
	QByteArray					 mTelnetSequence;
	QTimer						 mTimer;
	telnet_t					*mTelnet;
	QVector<telnet_telopt_t>	 mOptions;
	Connection::LineMode		 mLineMode;
	bool						 mTelnetOptionsSent;
	bool						 mTelnetOptionsReceived;
	bool						 mLocalEcho;

	bool						 mDataReceived;
	bool						 mWebSocketActive;
	bool						 mWebSocketHeader;
	QString                      mWebSocketAccept;
	QByteArray					 mWebSocketBuffer;
	QString						 mWebSocketOrigin;
	QString						 mWebSocketProtocol;
};

#endif // LISTENERTELNETSOCKET_H
