#ifndef OBJECTLOGIC_H
#define OBJECTLOGIC_H

#include "mooglobal.h"

class lua_task;

class ObjectLogic
{
public:
	static ObjectId create( lua_task &pTask, ObjectId pUserId, ObjectId pParentId, ObjectId pOwnerId );
	static void chparent( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId, ObjectId pNewParentId );
	static void recycle( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId );
	static void move( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId, ObjectId pWhereId );

};

#endif // OBJECTLOGIC_H
