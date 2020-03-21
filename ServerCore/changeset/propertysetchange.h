#ifndef PROPERTYSETCHANGE_H
#define PROPERTYSETCHANGE_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class PropertySetChange : public Change
{
public:
	PropertySetChange( Property *pProperty, bool pValue )
		: mProperty( pProperty )
	{
		mOldValue = mProperty->change();

		mProperty->setChange( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mProperty->setChange( mOldValue );
	}

private:
	Property	*mProperty;
	bool		 mOldValue;
};

}

#endif // PROPERTYSETCHANGE_H
