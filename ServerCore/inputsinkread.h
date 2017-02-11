#ifndef INPUTSINKREAD_H
#define INPUTSINKREAD_H

#include <QVariantMap>
#include <QVariantList>

#include "mooglobal.h"

#include "inputsink.h"

class Connection;
class Object;
class Verb;

class InputSinkRead : public InputSink
{
public:
	InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs );

	virtual bool input( const QString &pData );

private:
	void processAnsiSequence( const QByteArray &pData );

private:
	Connection		*mConnection;
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
