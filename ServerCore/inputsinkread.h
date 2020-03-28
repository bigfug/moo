#ifndef INPUTSINKREAD_H
#define INPUTSINKREAD_H

#include <QVariantMap>
#include <QVariantList>

#include "mooglobal.h"
#include "task.h"

#include "inputsink.h"

class Connection;
class Object;
class Verb;

class InputSinkRead : public InputSink
{
public:
//	InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs );

	InputSinkRead( Connection *C, const Task &pTask, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs = QVariantMap(), QVariantList pVerbArgs = QVariantList() );

	InputSinkRead( Connection *C, const Task &pTask, QVariantMap pReadArgs = QVariantMap(), QVariantList pVerbArgs = QVariantList() );

	virtual bool input( const QString &pData );

private:
	void processAnsiSequence( const QByteArray &pData );

private:
	Connection		*mConnection;
	Task			 mTask;
	ObjectId		 mObjectId;
	QString			 mVerbName;
	QString			 mInput;
	QVariantMap		 mReadArgs;
	QVariantList	 mVerbArgs;

	int				 mAnsiEsc;
	QByteArray		 mAnsiSeq;
	int				 mAnsiPos;
};


#endif // INPUTSINKREAD_H
