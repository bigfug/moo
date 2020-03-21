#ifndef PROPERTYSETWRITE_H
#define PROPERTYSETWRITE_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class PropertySetWrite : public Change
{
public:
	PropertySetWrite( Property *pProperty, bool pValue )
		: mProperty( pProperty )
	{
		mOldValue = mProperty->write();

		mProperty->setWrite( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mProperty->setWrite( mOldValue );
	}

private:
	Property	*mProperty;
	bool		 mOldValue;
};

}

#endif // PROPERTYSETWRITE_H
