
#include "tst_servertest.h"

#include "task.h"
#include "taskentry.h"

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

void ServerTest::taskSchedule( void )
{
	TaskEntry			TE;
	TaskEntrySchedule	TS;

	QDateTime			DT;

	DT.setDate( QDate( 2020, 04, 17 ) );
	DT.setTime( QTime( 14, 22, 48 ) );

	qint64				CT = DT.toMSecsSinceEpoch();

	TS.mYear       = "2021";
	TS.mMonth      = "*";
	TS.mDayOfMonth = "*";
	TS.mDayOfWeek  = "*";
	TS.mHour       = "*";
	TS.mMinute     = "*";

	TE.setSchedule( TS );

	TE.updateTimestampFromSchedule( CT );

	QDateTime			DTT;

	DTT.setMSecsSinceEpoch( TE.timestamp() );

	QCOMPARE( DTT.date().year(), 2021 );
	QCOMPARE( DTT.date().month(), 1 );
	QCOMPARE( DTT.date().day(), 1 );
	QCOMPARE( DTT.time().hour(), 0 );
	QCOMPARE( DTT.time().minute(), 0 );
	QCOMPARE( DTT.time().second(), 0 );

	//--

	TS.mYear       = "*";
	TS.mMonth      = "5";
	TS.mDayOfMonth = "*";
	TS.mDayOfWeek  = "*";
	TS.mHour       = "*";
	TS.mMinute     = "*";

	TE.setSchedule( TS );

	TE.updateTimestampFromSchedule( CT );

	DTT.setMSecsSinceEpoch( TE.timestamp() );

	QCOMPARE( DTT.date().year(), 2020 );
	QCOMPARE( DTT.date().month(), 5 );
	QCOMPARE( DTT.date().day(), 1 );
	QCOMPARE( DTT.time().hour(), 0 );
	QCOMPARE( DTT.time().minute(), 0 );
	QCOMPARE( DTT.time().second(), 0 );

	//--

	TS.mYear       = "*";
	TS.mMonth      = "7";
	TS.mDayOfMonth = "*";
	TS.mDayOfWeek  = "4-6";
	TS.mHour       = "7";
	TS.mMinute     = "12,45,58";

	TE.setSchedule( TS );

	TE.updateTimestampFromSchedule( CT );

	DTT.setMSecsSinceEpoch( TE.timestamp() );

	QCOMPARE( DTT.date().year(), 2020 );
	QCOMPARE( DTT.date().month(), 7 );
	QCOMPARE( DTT.date().day(), 2 );
	QCOMPARE( DTT.time().hour(), 7 );
	QCOMPARE( DTT.time().minute(), 12 );
	QCOMPARE( DTT.time().second(), 0 );
}
