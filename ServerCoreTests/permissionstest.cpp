#include "permissionstest.h"

#include "luatestdata.h"

/*
ProgrammerId = OBJECT_NONE
ProgrammerId = Wizard
ProgrammerId = Programmer
ProgrammerId = Player
ProgrammerId = Thing

O1.owner = OBJECT_NONE
O1.owner = Wizard
O1.owner = Programmer
O1.owner = Player
O1.owner = Thing

O1.r = true
O1.r = false

O1.w = true
O1.w = false

O1.x = true
O1.x = false

*/

PermissionsTest::PermissionsTest( void )
{
}

void PermissionsTest::propAdd_data()
{
	QList<ObjectId>		ProgrammerIds;
	QList<bool>			BoolList;
	int					TestNumber = 0;

	ProgrammerIds << OBJECT_NONE << 3 << 4;

	BoolList << true << false;

	QTest::addColumn<ObjectId>( "ProgrammerId" );
	QTest::addColumn<ObjectId>( "ObjectParentId" );
	QTest::addColumn<ObjectId>( "ObjectOwnerId" );
	QTest::addColumn<bool>( "ObjectRead" );
	QTest::addColumn<bool>( "ObjectWrite" );

	for( ObjectId ProgrammerId : ProgrammerIds )
	{
		for( ObjectId ObjectParentId : ProgrammerIds )
		{
			for( ObjectId ObjectOwnerId : ProgrammerIds )
			{
				for( bool ObjectRead : BoolList )
				{
					for( bool ObjectWrite : BoolList )
					{
						const QString	TestName = QString( "t%1" ).arg( ++TestNumber );

						QTest::newRow( TestName.toLatin1() ) << ProgrammerId << ObjectParentId << ObjectOwnerId << ObjectRead << ObjectWrite;
					}
				}
			}
		}
	}
}

void PermissionsTest::propAdd()
{
	QFETCH( ObjectId, ProgrammerId );
	QFETCH( ObjectId, ObjectParentId );
	QFETCH( ObjectId, ObjectOwnerId );
	QFETCH( bool, ObjectRead );
	QFETCH( bool, ObjectWrite );

	LuaTestData			 TD;
	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):propadd( 'test', 'testval' )" ).arg( O->id() );

	O->setParent( ObjectParentId );
	O->setOwner( ObjectOwnerId );
	O->setRead( ObjectRead );
	O->setWrite( ObjectWrite );

	// Call test

	TD.execute( CD );

	// Check result

	Property	*R = O->prop( "test" );

	qDebug() << "PRID =" << ProgrammerId << "PAID =" << ObjectParentId << "OWID =" << ObjectOwnerId << "OR =" << ObjectRead << "OW =" << ObjectWrite << "R =" << ( R == 0 ? "FAIL" : "OK" );
}

QTEST_GUILESS_MAIN( PermissionsTest )

#include "permissionstest.moc"
