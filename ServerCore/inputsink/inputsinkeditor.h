#ifndef INPUTSINKEDITOR_H
#define INPUTSINKEDITOR_H

#include "inputsink.h"

#include <editor.h>

#include "mooglobal.h"

class Connection;
class Object;
class Verb;

class InputSinkEditor : public QObject, public InputSink
{
	Q_OBJECT

public:
	InputSinkEditor( Connection *C, ObjectId pObjectId, QString pVerbName, QStringList pText );

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

	void test( void );

	void save( void );

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mVerbName;
	Editor			 mEditor;
};

#endif // INPUTSINKEDITOR_H
