#ifndef LISTENERTELNETSOCKET_H
#define LISTENERTELNETSOCKET_H

#include "mooglobal.h"
#include <QObject>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

#include "libtelnet.h"

#include "listenersocket.h"

class ListenerSocketTelnet : public ListenerSocket
{
	Q_OBJECT

public:
	explicit ListenerSocketTelnet( QObject *pParent );

	virtual ~ListenerSocketTelnet( void );

	void setOptions( const QVector<telnet_telopt_t> &pOptions )
	{
		mOptions = pOptions;
	}

	bool echo( void ) const;

	virtual bool isOpen( void ) const = 0;

	void start( void ); //ListenerSocketTelnet *pThis );

private:
	void sendData( const QByteArray &pData );

	void processInput( const QByteArray &pData );

	static void telnetEventHandlerStatic( telnet_t *telnet, telnet_event_t *event, void *user_data );

	void telnetEventHandler( telnet_event_t *event );

public slots:
	void textInput( const QString &pText );

	void inputTimeout( void );

	void setLineMode( Connection::LineMode pLineMode );

	void sendGMCP( const QByteArray &pGMCP );

	virtual void close( void ) = 0;

protected slots:
	virtual qint64 write( const QByteArray &A ) = 0;
	virtual qint64 write( const char *p, qint64 l ) = 0;

	void read( QByteArray A );

	void stopTimer( void );

signals:
	void textOutput( const QString &pText );
	void lineModeSupported( bool pLineModeSupport );
	void terminalSizeChanged( const QSize &pSize );

private:
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
};

#endif // LISTENERTELNETSOCKET_H
