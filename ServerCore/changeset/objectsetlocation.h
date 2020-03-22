#ifndef OBJECTSETLOCATION_H
#define OBJECTSETLOCATION_H

#include "../object.h"
#include "../objectmanager.h"

#include "change.h"

namespace change {

class ObjectSetLocation : public Change
{
public:
	ObjectSetLocation( Object *pObject, ObjectId pLocationId )
		: mObject( pObject )
	{
		mOldLocation = mObject->location();

		mObject->move( ObjectManager::o( pLocationId ) );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->move( ObjectManager::o( mOldLocation ) );
	}

private:
	Object		*mObject;
	ObjectId	 mOldLocation;
};

}

#endif // OBJECTSETLOCATION_H
