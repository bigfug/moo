#ifndef OBJECTSETPARENT_H
#define OBJECTSETPARENT_H


#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetParent : public Change
{
public:
	ObjectSetParent( Object *pObject, ObjectId pNewParentId )
		: mObject( pObject ), mNewParent( pNewParentId )
	{
		mOldParent = pObject->parent();

		mObject->setParent( mNewParent );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setParent( mOldParent );
	}

private:
	Object		*mObject;
	ObjectId	 mNewParent;
	ObjectId	 mOldParent;
};

}

#endif // OBJECTSETPARENT_H
