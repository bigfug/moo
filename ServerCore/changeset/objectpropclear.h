#ifndef OBJECTPROPCLEAR_H
#define OBJECTPROPCLEAR_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class ObjectPropClear : public Change
{
public:
	ObjectPropClear( Object *pObject, QString pPropName )
		: mObject( pObject ), mPropName( pPropName )
	{
		mOldProp = *mObject->prop( mPropName );

		mObject->propClear( mPropName );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->propSet( mPropName, mOldProp.value() );
	}

private:
	Object		*mObject;
	QString		 mPropName;
	Property	 mOldProp;
};

}

#endif // OBJECTPROPCLEAR_H
