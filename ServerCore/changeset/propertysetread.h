#ifndef PROPERTYSETREAD_H
#define PROPERTYSETREAD_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class PropertySetRead : public Change
{
public:
	PropertySetRead( Property *pProperty, bool pValue )
		: mProperty( pProperty )
	{
		mOldValue = mProperty->read();

		mProperty->setRead( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mProperty->setRead( mOldValue );
	}

private:
	Property	*mProperty;
	bool		 mOldValue;
};

}

#endif // PROPERTYSETREAD_H
