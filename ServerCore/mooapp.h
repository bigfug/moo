#ifndef MOOAPP_H
#define MOOAPP_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include <lua.hpp>

#include "taskentry.h"

class mooApp : public QObject
{
    Q_OBJECT

public:
	explicit mooApp( const QString &pDataFileName = "moo.dat", QObject *pParent = 0 );

	virtual ~mooApp();

signals:
	void textOutput( const QString &pText );

	void frameStart( void );
	void frameEnd( void );

	void frameStart( qint64 pTimeStamp );
	void frameEnd( qint64 pTimeStamp );

public slots:
	void doTask( TaskEntry &pTask );
	void cleanup( QObject *pObject = 0 );

private slots:
	void doOutput( const QString &pText );

	void taskReady( void );

private:
	int					 mTimerId;
	const QString		 mDataFileName;
	QTimer				 mTimer;

	static void streamCallback( const QString &pText, void *pUserData );

	void timerEvent( QTimerEvent *pEvent );
};

#endif // MOOAPP_H
