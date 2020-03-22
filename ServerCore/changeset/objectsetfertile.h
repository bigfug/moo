#ifndef OBJECTSETFERTILE_H
#define OBJECTSETFERTILE_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetFertile : public Change
{
public:
	ObjectSetFertile( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->fertile();

		mObject->setFertile( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setFertile( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETFERTILE_H
