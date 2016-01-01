
#include "tst_servertest.h"

#include "task.h"


void ServerTest::taskDefaults( void )
{
	Task			T;

	QVERIFY( T.args().isEmpty() );
	QVERIFY( T.argstr().isEmpty() );
	QVERIFY( T.caller() == OBJECT_NONE );
	QVERIFY( T.command().isEmpty() );
	QVERIFY( T.directObjectId() == OBJECT_NONE );
	QVERIFY( T.directObjectName().isEmpty() );
	QVERIFY( T.indirectObjectId() == OBJECT_NONE );
	QVERIFY( T.indirectObjectName().isEmpty() );
	QVERIFY( T.object() == OBJECT_NONE );
	QVERIFY( T.player() == OBJECT_NONE );
	QVERIFY( T.preposition().isEmpty() );
	QVERIFY( T.verb().isEmpty() );
}

void ServerTest::taskGetSet( void )
{
}


