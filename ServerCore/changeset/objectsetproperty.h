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
		mHasProp = ( mObject->prop( mPrpNam ) ? true : false );

		mOldValue = mObject->propValue( mPrpNam );

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
		if( mHasProp )
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
	QString		 mPrpNam;
	QVariant	 mOldValue;
	bool		 mHasProp;
};

}

#endif // OBJECTSETPROPERTY_H
