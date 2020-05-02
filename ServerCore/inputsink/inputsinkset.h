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
	QString			 mPropName;
	QStringList		 mData;
};

#endif // INPUTSINKSET_H
