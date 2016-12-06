#ifndef CURSESREADER_H
#define CURSESREADER_H

#include <QThread>

#include <curses.h>

class CursesReader : public QThread
{
	Q_OBJECT

public:
	CursesReader( void );

signals:
	void input( const QString &pData );

	void resized( int w, int h );

public slots:
	void output( const QString &pData );

	// QThread interface
protected:
	virtual void run() Q_DECL_OVERRIDE;

private:
	WINDOW			*mWindow;
};

#endif // CURSESREADER_H
