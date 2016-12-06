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
	InputSinkEditor( Connection *C, Object *O, Verb *V, const QString &pVerbName, QStringList pText );

	virtual bool input( const QString &pData );

private slots:
	void output( const QString &pData );

	void test( void );

	void save( void );

private:
	Connection		*mConnection;
	Object			*mObject;
	Verb			*mVerb;
	QString			 mVerbName;
	Editor			 mEditor;
};

#endif // INPUTSINKEDITOR_H
