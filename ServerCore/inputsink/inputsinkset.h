#ifndef INPUTSINKSET_H
#define INPUTSINKSET_H

#include <QStringList>

#include "inputsink.h"
#include "mooglobal.h"

class Connection;
class Object;
class Property;

class InputSinkSet : public InputSink
{
public:
	InputSinkSet( Connection *C, ObjectId pObjectId, QString pPropName );

	virtual bool input( const QString &pData );

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mPropName;
	QStringList		 mData;
};

#endif // INPUTSINKSET_H
