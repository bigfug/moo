#ifndef INPUTSINKPROGRAM_H
#define INPUTSINKPROGRAM_H

#include <QStringList>

#include "inputsink.h"

class Connection;
class Object;
class Verb;

class InputSinkProgram : public InputSink
{
public:
	InputSinkProgram( Connection *C, Object *O, Verb *V, const QString &pVerbName );

	virtual bool input( const QString &pData );

private:
	Connection		*mConnection;
	Object			*mObject;
	Verb			*mVerb;
	QString			 mVerbName;
	QStringList		 mProgram;
};

#endif // INPUTSINKPROGRAM_H
