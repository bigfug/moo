#ifndef LUATESTDATA_H
#define LUATESTDATA_H

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

class LuaTestData
{
public:
	LuaTestData( void );

	~LuaTestData( void );

	void initTask( const QString &pCmd, ObjectId pProgrammerId );

	ObjectManager		&OM;
	ConnectionManager	&CM;
	qint64				 TimeStamp;
	ConnectionId		 CID;
	Connection			*Con;

	Object			*Programmer;

	QString			 CMD;
	TaskEntry		 TE;
	lua_task		 *Com;
};

#endif // LUATESTDATA_H
