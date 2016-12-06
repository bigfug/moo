#ifndef INPUTSINKEDITOR_H
#define INPUTSINKEDITOR_H

#include "inputsink.h"

#include <editor.h>

class Connection;
class Object;
class Verb;

class InputSinkEditor : public QObject, public InputSink
{
	Q_OBJECT

public:
	InputSinkEditor( Connection *C, Object *O, Verb *V, QStringList pText);

	virtual bool input( const QString &pData );

private slots:
	void output( const QString &pData );

private:
	Connection		*mConnection;
	Object			*mObject;
	Verb			*mVerb;
	Editor			 mEditor;
};

#endif // INPUTSINKEDITOR_H
