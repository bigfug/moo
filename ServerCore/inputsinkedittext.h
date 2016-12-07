#ifndef INPUTSINKEDITTEXT_H
#define INPUTSINKEDITTEXT_H

#include "inputsink.h"

#include <editor.h>

class Connection;
class Object;
class Property;

class InputSinkEditText : public QObject, public InputSink
{
	Q_OBJECT

public:
	InputSinkEditText( Connection *C, Object *O, Property *P, QStringList pText );

	virtual bool input( const QString &pData );

private slots:
	void output( const QString &pData );

	void save( void );

private:
	Connection		*mConnection;
	Object			*mObject;
	Property		*mProperty;
	Editor			 mEditor;
};

#endif // INPUTSINKEDITTEXT_H
