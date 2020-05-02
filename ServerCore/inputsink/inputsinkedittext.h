#ifndef INPUTSINKEDITTEXT_H
#define INPUTSINKEDITTEXT_H

#include "inputsink.h"

#include "mooglobal.h"

#include <editor.h>

class Connection;
class Object;
class Property;

class InputSinkEditText : public QObject, public InputSink
{
	Q_OBJECT

public:
	InputSinkEditText( Connection *C, ObjectId pObjectId, QString pPropName, QStringList pText );

	virtual bool input( const QString &pData ) Q_DECL_OVERRIDE;

	virtual bool output( const QString &pData ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pData )

		return( true );
	}

	virtual bool screenNeedsReset( void ) const Q_DECL_OVERRIDE
	{
		return( true );
	}

	virtual Connection::LineMode lineMode( void ) const Q_DECL_OVERRIDE
	{
		return( Connection::REALTIME );
	}

private slots:
	void editorOutput( const QString &pData );

	void save( void );

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mPropName;
	Editor			 mEditor;
};

#endif // INPUTSINKEDITTEXT_H
