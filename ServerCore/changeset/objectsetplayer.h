#ifndef OBJECTSETPLAYER_H
#define OBJECTSETPLAYER_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetPlayer : public Change
{
public:
	ObjectSetPlayer( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->player();

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
		mObject->setPlayer( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETPLAYER_H
