#ifndef INPUTSINKREAD_H
#define INPUTSINKREAD_H

#include "mooglobal.h"

#include "inputsink.h"

class Connection;
class Object;
class Verb;

class InputSinkRead : public InputSink
{
public:
	InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName );

	virtual bool input( const QString &pData );

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mVerbName;
	QString			 mInput;
};


#endif // INPUTSINKREAD_H
