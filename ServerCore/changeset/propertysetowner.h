#ifndef PROPERTYSETOWNER_H
#define PROPERTYSETOWNER_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class PropertySetOwner : public Change
{
public:
	PropertySetOwner( Property *pProperty, ObjectId pOwnerId )
		: mProperty( pProperty )
	{
		mOldOwner = mProperty->owner();

		mProperty->setOwner( pOwnerId );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mProperty->setOwner( mOldOwner );
	}

private:
	Property	*mProperty;
	ObjectId	 mOldOwner;
};

}

#endif // PROPERTYSETOWNER_H
