#ifndef OBJECTCREATE_H
#define OBJECTCREATE_H

#include "../object.h"
#include "../objectmanager.h"

#include "change.h"

namespace change {

class ObjectCreate : public Change
{
public:
	ObjectCreate( ObjectId pObjectId )
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
		ObjectManager::instance()->recycle( mObjectId );
	}

private:
	ObjectId		 mObjectId;
};

}

#endif // OBJECTCREATE_H
