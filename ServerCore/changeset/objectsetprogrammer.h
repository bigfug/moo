#ifndef OBJECTSETPROGRAMMER_H
#define OBJECTSETPROGRAMMER_H
#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetProgrammer : public Change
{
public:
	ObjectSetProgrammer( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->programmer();

		mObject->setProgrammer( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setProgrammer( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETPROGRAMMER_H
