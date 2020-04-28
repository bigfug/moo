#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QObject>
#include <QByteArray>

class LineEdit : public QObject
{
	Q_OBJECT

public:
	explicit LineEdit( QObject *pParent = nullptr );

public slots:
	void dataInput( const QByteArray &pData );		// characters received from user

	void setEditLinePosition( int y );

	void setTerminalWidth( int w );

	void redraw( void );

	void setSecretChar( QChar c )
	{
		mSecretChar = c;
	}

signals:
	void dataOutput( const QByteArray &pData );		// characters to send to user

	void lineOutput( const QByteArray &pData );		// when user presses return

private:
	void processAnsiSequence( const QByteArray &pData, QByteArray &pTemp );
	void processFunctionKey( int pAnsiCode, QByteArray &pTemp );

	void processCursorLeft( QByteArray &pTemp );
	void processCursorRight( QByteArray &pTemp );
	void processDelete( QByteArray &pBuffer );

	void write( const char *pData );
	void write( const QString &pData );
	void write( const QByteArray &pData );

private:
	QByteArray		 mLineBuffer;
	quint8			 mLastChar;
	int				 mEditLinePosition;
	int				 mTerminalWidth;
	QChar			 mSecretChar;
	QByteArray		 mPrompt;

	int				 mAnsiEsc;
	QByteArray		 mAnsiSeq;
	int				 mAnsiPos;
};

#endif // LINEEDIT_H
