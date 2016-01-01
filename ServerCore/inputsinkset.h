#ifndef INPUTSINKSET_H
#define INPUTSINKSET_H

#include <QStringList>

#include "inputsink.h"

class Connection;
class Object;
class Property;

class InputSinkSet : public InputSink
{
public:
	InputSinkSet( Connection *C, Object *O, Property *P, const QString &pPropName );

	virtual bool input( const QString &pData );

private:
	Connection		*mConnection;
	Object			*mObject;
	Property		*mProperty;
	QString			 mPropName;
	QStringList		 mData;
};

#endif // INPUTSINKSET_H
