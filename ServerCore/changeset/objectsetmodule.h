#ifndef OBJECTSETMODULE_H
#define OBJECTSETMODULE_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetModule : public Change
{
public:
	ObjectSetModule( Object *pObject, ObjectId pModuleId )
		: mObject( pObject )
	{
		mOldModule = mObject->owner();

		mObject->setModule( pModuleId );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setModule( mOldModule );
	}

private:
	Object		*mObject;
	ObjectId	 mOldModule;
};

}

#endif // OBJECTSETMODULE_H
