#ifndef OBJECTSETREAD_H
#define OBJECTSETREAD_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetRead : public Change
{
public:
	ObjectSetRead( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->read();

		mObject->setRead( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setRead( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETREAD_H
