#ifndef OBJECTSETWRITE_H
#define OBJECTSETWRITE_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetWrite : public Change
{
public:
	ObjectSetWrite( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->write();

		mObject->setWrite( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setWrite( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETWRITE_H
