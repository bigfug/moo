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

	void setCommandHistoryCount( int pCount )
	{
		mCommandHistoryCount = pCount;
	}

	void setCommandHistoryEntry( const QByteArray &pData );

signals:
	void dataOutput( const QByteArray &pData );		// characters to send to user

	void lineOutput( const QByteArray &pData );		// when user presses return

	void commandHistoryLookup( int pIndex );

protected:
	void redraw( QByteArray &pOutput );

	void processCursorToStartOfLine( QByteArray &pOutput );
	void processCursorToEndOfLine( QByteArray &pOutput );

	void processAnsiSequence( const QByteArray &pData, QByteArray &pOutput );
	void processFunctionKey( int pAnsiCode, QByteArray &pOutput );

	void processCharacter( QChar ch, QByteArray &pOutput );
	void processAltCharacter( QChar ch, QByteArray &pOutput );

	void processCursorUp( QByteArray &pOutput );
	void processCursorDown( QByteArray &pOutput );

	void processCursorLeft( QByteArray &pOutput );
	void processCursorRight( QByteArray &pOutput );
	void processDelete( QByteArray &pOutput );
	void processBackspace( QByteArray &pOutput );

	void processEnter( QByteArray &pOutput );

	void processForwardOneWord( QByteArray &pOutput );
	void processBackwardOneWord( QByteArray &pOutput );
	void processDeleteOneWord( QByteArray &pOutput );

	void write( const char *pData );
	void write( const QString &pData );
	void write( const QByteArray &pData );

private:
	QByteArray		 mLineBuffer;
	QChar			 mLastChar;
	int				 mEditLinePosition;
	int				 mTerminalWidth;
	QChar			 mSecretChar;
	QByteArray		 mPrompt;
	int				 mCommandHistoryCount;
	int				 mCommandHistoryPosition;
	QByteArray		 mCommandHistoryEntry;
	QByteArray		 mCommandHistoryLineBuffer;

	int				 mAnsiEsc;
	QByteArray		 mAnsiSeq;
	int				 mAnsiPos;
};

#endif // LINEEDIT_H
