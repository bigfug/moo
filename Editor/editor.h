#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QStringList>
#include <QMap>

class Editor : public QObject
{
	Q_OBJECT

public:
	Editor( QObject *pParent = Q_NULLPTR );

	virtual ~Editor( void ) {}

	QSize size( void ) const
	{
		return( mWindowSize );
	}

	int width( void ) const
	{
		return( mWindowSize.width() );
	}

	int height( void ) const
	{
		return( mWindowSize.height() );
	}

	bool hasQuit( void ) const
	{
		return( mQuit );
	}

	void addControlSlot( quint8 pASCII, QObject *pObject, QString pSlot );
	void remControlSlot( quint8 pASCII );

	QStringList text( void ) const
	{
		return( mText );
	}

	void setQuit( bool pQuit )
	{
		mQuit = pQuit;
	}

public slots:
	void setText( QStringList pText );

	void setSize( int w, int h );
	void setSize( const QSize &pSize );

	void input( const QString &pData );

	void setCursorScreenPosition( int x, int y );
	void setCursorScreenPosition( const QPoint &pP );

	void redraw( void );

	void clearScreen( void );

	void setStatusMessage( QString pStatusMessage );

signals:
	void output( const QString &pData );

	void quit( void );

	void sizeChanged( int w, int h );

private:
	void processANSI( QChar pC );
	void processCTRL( QChar pC );
	void processTEXT( QChar pC );

	void drawInfo( void );

	void drawStatusMessage( void );

	void updateCursorScreenPosition( void );

	void drawText( int pStart = 0 );

private:
	QSize			mWindowSize;
	QPoint			mCursorScreenPosition;
	QPoint			mCursorTextPosition;
	QPoint			mTextPosition;
	QStringList		mText;
	int				mANSI;
	QString			mANSIArg;
	QStringList		mANSIArgs;
	bool			mQuit;

	typedef QMap<quint8,QPair<QObject *,QString>>	ObjectSlotMap;

	ObjectSlotMap	mSlotMap;

	QString			 mStatusMessage;
};

#endif // EDITOR_H
