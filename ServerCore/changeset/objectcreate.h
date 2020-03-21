#ifndef OBJECTCREATE_H
#define OBJECTCREATE_H

#include "../lua_task.h"
#include "../connection.h"

#include "change.h"

namespace change {

class ObjectCreate : public Change
{
public:
	ObjectCreate( lua_task &pTask, ObjectId pUserId, ObjectId pParentId, ObjectId pOwnerId, Connection *pConnection = Q_NULLPTR )
	{

	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
	}
};

}

#endif // OBJECTCREATE_H
