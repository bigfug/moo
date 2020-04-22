#ifndef LUATESTDATA_H
#define LUATESTDATA_H

#include "mooglobal.h"

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

	lua_task execute( const QString &pCmd, bool pElevated = true );

	lua_task task( const QString &pCmd, bool pElevated = true );

	lua_task task( const QString &pCmd, ObjectId pProgrammerId, bool pElevated = true );

	lua_task eval( const QString &pCmd, bool pElevated = true );

	lua_task eval( const QString &pCmd, ObjectId pProgrammerId, bool pElevated = true );

	void process( const QString &pCmd, bool pElevated = true );

	void process( const QString &pCmd, ObjectId pProgrammerId, bool pElevated = true );

	inline ObjectId programmerId( void ) const
	{
		return( Programmer ? Programmer->id() : OBJECT_NONE );
	}

public:
	ObjectManager		&OM;
	ConnectionManager	&CM;
	qint64				 TimeStamp;
	ConnectionId		 CID;
	Connection			*Con;

	Object				*Programmer;
};

class LuaTestObject : public QObject
{
	Q_OBJECT

public:
	static ConnectionId initLua( qint64 pTimeStamp );

private slots:
	void initTestCase( void );

	void cleanupTestCase( void );
};

#endif // LUATESTDATA_H
