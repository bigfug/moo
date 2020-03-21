#ifndef OBJECTALIASADD_H
#define OBJECTALIASADD_H

#include "../object.h"
#include "../property.h"

#include "change.h"

namespace change {

class ObjectAliasAdd : public Change
{
public:
	ObjectAliasAdd( Object *pObject, QString pAlias )
		: mObject( pObject ), mAlias( pAlias )
	{
		mObject->aliasAdd( mAlias );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->aliasDelete( mAlias );
	}

private:
	Object		*mObject;
	QString		 mAlias;
};

}

#endif // OBJECTALIASADD_H
