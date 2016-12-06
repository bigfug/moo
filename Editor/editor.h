#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QStringList>

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

public slots:
	void setText( QStringList pText );

	void setSize( int w, int h );
	void setSize( const QSize &pSize );

	void input( const QString &pData );

	void setCursorScreenPosition( int x, int y );

	void redraw( void );

signals:
	void output( const QString &pData );

	void quit( void );

	void sizeChanged( int w, int h );

private:
	void processANSI( QChar pC );
	void processCTRL( QChar pC );
	void processTEXT( QChar pC );

	void clear( void );

	void drawInfo( void );

private:
	QSize			mWindowSize;
	QPoint			mCursorScreenPosition;
	QPoint			mCursorTextPosition;
	QStringList		mText;
	int				mANSI;
	QString			mANSIArg;
	QStringList		mANSIArgs;
	bool			mQuit;
};

#endif // EDITOR_H
