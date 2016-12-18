#ifndef INPUTSINKPROGRAM_H
#define INPUTSINKPROGRAM_H

#include <QStringList>

#include "mooglobal.h"

#include "inputsink.h"

class Connection;
class Object;
class Verb;

class InputSinkProgram : public InputSink
{
public:
	InputSinkProgram( Connection *C, ObjectId pObjectId, QString pVerbName );

	virtual bool input( const QString &pData );

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mVerbName;
	QStringList		 mProgram;
};

#endif // INPUTSINKPROGRAM_H
