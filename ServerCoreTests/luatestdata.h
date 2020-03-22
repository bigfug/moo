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

	lua_task execute( const QString &pCmd );

	lua_task task( const QString &pCmd );

	lua_task task( const QString &pCmd, ObjectId pProgrammerId );

	lua_task eval( const QString &pCmd );

	lua_task eval( const QString &pCmd, ObjectId pProgrammerId );

	void process( const QString &pCmd );

	void process( const QString &pCmd, ObjectId pProgrammerId );

	inline ObjectId programmerId( void ) const
	{
		return( Programmer->id() );
	}

public:
	ObjectManager		&OM;
	ConnectionManager	&CM;
	qint64				 TimeStamp;
	ConnectionId		 CID;
	Connection			*Con;

	Object				*Programmer;
};

#endif // LUATESTDATA_H
