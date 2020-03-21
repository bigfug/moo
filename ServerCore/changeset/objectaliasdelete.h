#ifndef OBJECTALIASDELETE_H
#define OBJECTALIASDELETE_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class ObjectAliasDelete : public Change
{
public:
	ObjectAliasDelete( Object *pObject, QString pAlias )
		: mObject( pObject ), mAlias( pAlias )
	{
		mObject->aliasDelete( mAlias );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->aliasAdd( mAlias );
	}

private:
	Object		*mObject;
	QString		 mAlias;
};

}

#endif // OBJECTALIASDELETE_H
