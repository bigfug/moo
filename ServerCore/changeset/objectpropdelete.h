#ifndef OBJECTPROPDELETE_H
#define OBJECTPROPDELETE_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class ObjectPropDelete : public Change
{
public:
	ObjectPropDelete( Object *pObject, QString pPropName )
		: mObject( pObject ), mPropName( pPropName )
	{
		mOldProp = *mObject->prop( mPropName );

		mObject->propDelete( mPropName );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->propAdd( mPropName, mOldProp );
	}

private:
	Object		*mObject;
	QString		 mPropName;
	Property	 mOldProp;
};

}
#endif // OBJECTPROPDELETE_H
