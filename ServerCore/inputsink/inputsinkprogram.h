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

	virtual bool input( const QString &pData ) Q_DECL_OVERRIDE;

	virtual bool output( const QString &pData ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pData )

		return( true );
	}

	virtual bool screenNeedsReset( void ) const Q_DECL_OVERRIDE
	{
		return( false );
	}

	virtual Connection::LineMode lineMode( void ) const Q_DECL_OVERRIDE
	{
		return( Connection::EDIT );
	}

private:
	Connection		*mConnection;
	ObjectId		 mObjectId;
	QString			 mVerbName;
	QStringList		 mProgram;
};

#endif // INPUTSINKPROGRAM_H
