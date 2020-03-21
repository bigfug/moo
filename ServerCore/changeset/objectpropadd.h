#ifndef OBJECTPROPADD_H
#define OBJECTPROPADD_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class ObjectPropAdd : public Change
{
public:
	ObjectPropAdd( Object *pObject, QString pPropName, Property pPropInit )
		: mObject( pObject ), mPropName( pPropName )
	{
		mObject->propAdd( mPropName, pPropInit );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->propDelete( mPropName );
	}

private:
	Object		*mObject;
	QString		 mPropName;
};

}

#endif // OBJECTPROPADD_H
