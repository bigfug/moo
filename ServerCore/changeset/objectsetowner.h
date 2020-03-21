#ifndef OBJECTSETOWNER_H
#define OBJECTSETOWNER_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetOwner : public Change
{
public:
	ObjectSetOwner( Object *pObject, ObjectId pOwnerId )
		: mObject( pObject )
	{
		mOldOwner = mObject->owner();

		mObject->setOwner( pOwnerId );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setOwner( mOldOwner );
	}

private:
	Object		*mObject;
	ObjectId	 mOldOwner;
};

}

#endif // OBJECTSETOWNER_H
