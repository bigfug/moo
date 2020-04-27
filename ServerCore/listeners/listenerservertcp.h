#ifndef LISTENERTELNET_H
#define LISTENERTELNET_H

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>

#include "mooglobal.h"
#include "listenerserver.h"

class SslServer : public QTcpServer
{
	Q_OBJECT

public:
	SslServer( QObject *parent = Q_NULLPTR )
		: QTcpServer( parent )
	{

	}

	virtual ~SslServer( void ) {}

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

class ListenerServerTCP : public ListenerServer
{
	Q_OBJECT

public:
	explicit ListenerServerTCP( ObjectId pObjectId, quint16 pPort, QObject *pParent = 0 );

	virtual ~ListenerServerTCP( void ) {}

private slots:
	void newConnection( void );

private:
	SslServer					 mServer;
	ObjectId					 mListeningObjectId;
};

#endif // LISTENERTELNET_H
