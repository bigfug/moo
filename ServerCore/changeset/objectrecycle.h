#ifndef OBJECTRECYCLE_H
#define OBJECTRECYCLE_H

#include "../object.h"
#include "../objectmanager.h"

#include "change.h"

namespace change {

class ObjectRecycle : public Change
{
public:
	ObjectRecycle( ObjectId pObjectId )
		: mObjectId( pObjectId )
	{
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		ObjectManager::instance()->restore( mObjectId );
	}

private:
	ObjectId		 mObjectId;
};

}

#endif // OBJECTRECYCLE_H
