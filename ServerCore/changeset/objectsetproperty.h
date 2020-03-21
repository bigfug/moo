#ifndef OBJECTSETPROPERTY_H
#define OBJECTSETPROPERTY_H

#include "../object.h"
#include "../lua_prop.h"
#include "objectmanager.h"

#include "change.h"

#include <iostream>

namespace change {

class ObjectSetProperty : public Change
{
public:
	ObjectSetProperty( Object *pObject, QString pPrpNam, QVariant pValue )
		: mObject( pObject ), mPrpNam( pPrpNam )
	{
		mProperty = mObject->prop( mPrpNam );

		if( mProperty )
		{
			mOldValue = mProperty->value();
		}

		mObject->propSet( mPrpNam, pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		if( mProperty )
		{
			mObject->propSet( mPrpNam, mOldValue );
		}
		else
		{
			mObject->propClear( mPrpNam );
		}
	}

private:
	Object		*mObject;
	Property	*mProperty;
	QString		 mPrpNam;
	QVariant	 mOldValue;
};

}

#endif // OBJECTSETPROPERTY_H
