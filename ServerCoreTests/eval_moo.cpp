#include "eval_moo.h"

void eval_moo::broadcast_data()
{
	QTest::addColumn<bool>( "elevated" );
	QTest::addColumn<bool>( "error" );

	QTest::newRow( "elevated" ) << true << false;
	QTest::newRow( "not elevated" ) << false << true;
}

void eval_moo::broadcast()
{
	QFETCH( bool, elevated );
	QFETCH( bool, error );

	LuaTestData		TD;

	lua_task		T = TD.execute( ";moo.broadcast()", elevated );

	QCOMPARE( T.error(), error );
}

void eval_moo::notify()
{
	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	lua_task		T = TD.execute( ";moo.notify( \"hello, world!\" )", false );

	QCOMPARE( T.error(), false );
}

void eval_moo::root()
{
	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = moo.root.id", false );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 1 );
}

void eval_moo::system()
{
	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = moo.system.id", false );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 0 );
}

void eval_moo::eval_data()
{
	QTest::addColumn<bool>( "programmer" );
	QTest::addColumn<bool>( "wizard" );
	QTest::addColumn<bool>( "elevated" );
	QTest::addColumn<ObjectId>( "owner" );
	QTest::addColumn<bool>( "error" );
	QTest::addColumn<int>( "result" );

	//                                     prg      wiz      elv      own  err     res

	QTest::newRow( "not programmer" )	<< false << false << false << 3 << true  << -1;
	QTest::newRow( "programmer" )		<< true	 << false << false << 3 << false << 23;
	QTest::newRow( "wizard" )			<< true  << true  << false << 3 << false << 23;
	QTest::newRow( "elevated" )			<< true  << true  << true  << 3 << false << 23;

	QTest::newRow( "not programmer" )	<< false << false << false << 2 << true  << -1;
	QTest::newRow( "programmer" )		<< true	 << false << false << 2 << true  << -1;
	QTest::newRow( "wizard" )			<< true  << true  << false << 2 << true  << -1;
	QTest::newRow( "elevated" )			<< true  << true  << true  << 2 << false << 23;
}

void eval_moo::eval()
{
	QFETCH( bool, programmer );
	QFETCH( bool, wizard );
	QFETCH( bool, elevated );
	QFETCH( ObjectId, owner );
	QFETCH( bool, error );
	QFETCH( int,  result );

	LuaTestData		TD;

	TD.Programmer->setProgrammer( programmer );
	TD.Programmer->setWizard( wizard );

	TD.Programmer->propAdd( "result", -1.0, owner );

	lua_task		T = TD.execute( ";moo.player.result = 23", elevated );

	QCOMPARE( T.error(), error );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), result );
}

void eval_moo::evalArgs_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<QString>( "pResult" );

	QTest::newRow( "player" ) << "player" << "#3";
	QTest::newRow( "caller" ) << "caller" << "#3";		// TODO: check this
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
