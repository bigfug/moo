#include "eval_moo.h"

void eval_moo::broadcast_data()
{
	QTest::addColumn<ObjectId>( "pObjectId" );
	QTest::addColumn<bool>( "error" );

//	QTest::newRow( "elevated" ) << OBJECT_SYSTEM << false;
	QTest::newRow( "not elevated" ) << 3 << true;
}

void eval_moo::broadcast()
{
	QFETCH( ObjectId, pObjectId );
	QFETCH( bool, error );

	LuaTestData		TD;

	lua_task		T = TD.execute( ";moo.broadcast()", pObjectId );

	QCOMPARE( T.error(), error );
}

void eval_moo::notify()
{
	LuaTestData		TD;

	lua_task		T = TD.execute( ";moo.notify( \"hello, world!\" )" );

	QCOMPARE( T.error(), false );
}

void eval_moo::root()
{
	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = moo.root.id" );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 1 );
}

void eval_moo::system()
{
	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = moo.system.id" );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 0 );
}

void eval_moo::eval_data()
{
	QTest::addColumn<bool>( "programmer" );
	QTest::addColumn<ObjectId>( "owner" );
	QTest::addColumn<bool>( "error" );
	QTest::addColumn<int>( "result" );

	//                                     prg      own  err     res

	QTest::newRow( "not programmer" )	<< false << 3 << true  << -1;
	QTest::newRow( "programmer" )		<< true	 << 3 << false << 23;
	QTest::newRow( "wizard" )			<< true  << 3 << false << 23;

	QTest::newRow( "not programmer" )	<< false << 2 << true  << -1;
	QTest::newRow( "programmer" )		<< true	 << 2 << true  << -1;
	QTest::newRow( "wizard" )			<< true  << 2 << true  << -1;
}

void eval_moo::eval()
{
	QFETCH( bool, programmer );
	QFETCH( ObjectId, owner );
	QFETCH( bool, error );
	QFETCH( int,  result );

	LuaTestData		TD;

	TD.Programmer->setProgrammer( programmer );

	TD.Programmer->propAdd( "result", -1.0, owner );

	lua_task		T = TD.execute( ";moo.player.result = 23" );

	QCOMPARE( T.error(), error );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), result );
}

void eval_moo::evalArgs_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<QString>( "pResult" );

	QTest::newRow( "player" ) << "player" << "#3";
	QTest::newRow( "caller" ) << "caller" << "#3";
	QTest::newRow( "object" ) << "object" << "#-1";
	QTest::newRow( "argstr" ) << "argstr" << "";
	QTest::newRow( "verb" ) << "verb" << "";
	QTest::newRow( "dobjstr" ) << "dobjstr" << "";
	QTest::newRow( "dobj" ) << "dobj" << "#-1";
	QTest::newRow( "prepstr" ) << "prepstr" << "";
	QTest::newRow( "iobjstr" ) << "iobjstr" << "";
	QTest::newRow( "iobj" ) << "iobj" << "#-1";
}

void eval_moo::evalArgs()
{
	QFETCH( QString, pType );
	QFETCH( QString, pResult );

	LuaTestData		TD;

	TD.Programmer->propAdd( "result", "no result", TD.programmerId() );

	lua_task		T = TD.execute( QString( ";moo.player.result = tostring( moo.%1 )" ).arg( pType ) );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toString(), pResult );
}

QTEST_GUILESS_MAIN( eval_moo )

#include "eval_moo.moc"
