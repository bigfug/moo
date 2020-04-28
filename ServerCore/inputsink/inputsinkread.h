#ifndef INPUTSINKREAD_H
#define INPUTSINKREAD_H

#include <QVariantMap>
#include <QVariantList>

#include "mooglobal.h"
#include "task.h"

#include "inputsink.h"
#include "lineedit.h"

class Connection;
class Object;
class Verb;

class InputSinkRead : public QObject, public InputSink
{
	Q_OBJECT

public:
//	InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs );

	InputSinkRead( Connection *C, const Task &pTask, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs = QVariantMap(), QVariantList pVerbArgs = QVariantList() );

	InputSinkRead( Connection *C, const Task &pTask, QVariantMap pReadArgs = QVariantMap(), QVariantList pVerbArgs = QVariantList() );

	virtual bool input( const QString &pData ) Q_DECL_OVERRIDE;

	virtual bool output( const QString &pData ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pData )

		return( false );
	}

	virtual bool screenNeedsReset( void ) const Q_DECL_OVERRIDE
	{
		return( false );
	}

	virtual Connection::LineMode lineMode( void ) const Q_DECL_OVERRIDE
	{
		return( Connection::REALTIME );
	}

private:
	void initialise( void );

private:
	Connection		*mConnection;
	Task			 mTask;
	ObjectId		 mObjectId;
	QString			 mVerbName;
//	QString			 mInput;
	QVariantMap		 mReadArgs;
	QVariantList	 mVerbArgs;
	LineEdit		 mLineEdit;

	bool			 mReadDone;
};


#endif // INPUTSINKREAD_H
