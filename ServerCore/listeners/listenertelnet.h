#ifndef LISTENERTELNET_H
#define LISTENERTELNET_H

#include "mooglobal.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

#include "libtelnet.h"

#include "listenerserver.h"

class ListenerTelnetServer : public QTcpServer
{
	Q_OBJECT

public:
	ListenerTelnetServer( QObject *parent = Q_NULLPTR )
		: QTcpServer( parent )
	{

	}

	virtual ~ListenerTelnetServer( void ) {}

	// QTcpServer interface
protected:
#if 0
	virtual void incomingConnection( qintptr handle ) Q_DECL_OVERRIDE
	{
		QSslSocket	*ServerSocket = new QSslSocket();

		if( ServerSocket->setSocketDescriptor( handle ) )
		{
			addPendingConnection( ServerSocket );

			connect( ServerSocket, &QSslSocket::encrypted, this, &ListenerTelnetServer::socketEncrypted );

			connect( ServerSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(socketSslErrors(QList<QSslError>)) );

			ServerSocket->startServerEncryption();
		}
		else
		{
			delete ServerSocket;
		}
	}
#endif

private slots:
	void socketEncrypted( void )
	{
		QSslSocket	*ServerSocket = qobject_cast<QSslSocket *>( sender() );

		if( ServerSocket )
		{
			qInfo() << "Socket Encrypted";
		}
	}

	void socketSslErrors( const QList<QSslError> &errors )
	{
		for( const QSslError &E : errors )
		{
			qWarning() << E.errorString();
		}
	}
};

class ListenerTelnet : public ListenerServer
{
	Q_OBJECT

public:
	explicit ListenerTelnet( ObjectId pObjectId, quint16 pPort, QObject *pParent = 0 );

	virtual ~ListenerTelnet( void ) {}

private slots:
	void newConnection( void );

private:
	ListenerTelnetServer		 mServer;
	ObjectId					 mListeningObjectId;
	QVector<telnet_telopt_t>	 mOptions;
};

#endif // LISTENERTELNET_H
