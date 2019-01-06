#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QSize>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QXmlDefaultHandler>

#include "mooglobal.h"
#include "taskentry.h"
#include "inputsink.h"

class XmlParser : private QXmlDefaultHandler
{
public:
	XmlParser( const QString &pText )
	{
		QXmlInputSource		XmlSrc;

		XmlSrc.setData( "<moo>" + pText + "</moo>" );

		QXmlSimpleReader	Reader;

		Reader.setContentHandler( this );
		Reader.setEntityResolver( this );

		Reader.parse( XmlSrc );
	}

	QString result( void ) const
	{
		return( mXML );
	}

	// QXmlContentHandler interface
public:
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) Q_DECL_OVERRIDE;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) Q_DECL_OVERRIDE;
	virtual QString errorString() const Q_DECL_OVERRIDE;
	virtual bool characters(const QString &ch) Q_DECL_OVERRIDE;
	virtual bool skippedEntity(const QString &name) Q_DECL_OVERRIDE;

private:
	QString				mXML;
	QStringList			mStyles;
};

class Connection : public QObject, private QXmlDefaultHandler
{
	Q_OBJECT

	explicit Connection( ConnectionId pConnectionId, QObject *pParent = Q_NULLPTR );

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

	QVariant cookie( const QString &pName );

signals:
	void taskOutput( TaskEntry &pTask );
	void textOutput( const QString &pText );
	void lineModeChanged( Connection::LineMode pLineMode );
	void gmcpOutput( const QByteArray &pGMCP );

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

	void setCookie( const QString &pName, QVariant pValue );

	void clearCookie( const QString &pName );

	void sendGMCP( const QByteArray &pGMCP );

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
	QMap<QString,QVariant>	 mCookies;
};

#endif // CONNECTION_H
