#include "eval_moo.h"

void eval_moo::broadcast()
{
	LuaTestData		TD;

	lua_task		T = TD.execute( ";moo.broadcast()", false );

	QCOMPARE( T.error(), true );
}

void eval_moo::broadcastElevated()
{
	LuaTestData		TD;

	lua_task		T = TD.execute( ";moo.broadcast()", true );

	QCOMPARE( T.error(), false );
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

void eval_moo::evalNotProgrammer()
{
	LuaTestData		TD;

	TD.Programmer->setProgrammer( false );
	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = 23", false );

	QCOMPARE( T.error(), true );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), -1 );
}

void eval_moo::evalProgrammer()
{
	LuaTestData		TD;

	TD.Programmer->setProgrammer( true );
	TD.Programmer->setWizard( false );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = 23", false );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 23 );
}

void eval_moo::evalElevated()
{
	LuaTestData		TD;

	TD.Programmer->setProgrammer( true );
	TD.Programmer->setWizard( true );

	TD.Programmer->propAdd( "result", -1.0, TD.programmerId() );

	lua_task		T = TD.execute( ";moo.player.result = 23", true );

	QCOMPARE( T.error(), false );

	QCOMPARE( TD.Programmer->propValue( "result" ).toInt(), 23 );
}

QTEST_GUILESS_MAIN( eval_moo )

#include "eval_moo.moc"
