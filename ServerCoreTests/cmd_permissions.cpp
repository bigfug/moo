#include "cmd_permissions.h"

void cmd_permissions::testArgs_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<QString>( "pResult" );

	QTest::newRow( "player" ) << "player" << "#3";
	QTest::newRow( "caller" ) << "caller" << "#3";
	QTest::newRow( "object" ) << "object" << "#3";
	QTest::newRow( "permissions" ) << "permissions" << "#1";
	QTest::newRow( "verb" ) << "verb" << "test";
	QTest::newRow( "argstr" ) << "argstr" << "";
	QTest::newRow( "dobjstr" ) << "dobjstr" << "";
	QTest::newRow( "dobj" ) << "dobj" << "#-1";
	QTest::newRow( "iobjstr" ) << "iobjstr" << "";
	QTest::newRow( "iobj" ) << "iobj" << "#-1";
	QTest::newRow( "prepstr" ) << "prepstr" << "";
}

void cmd_permissions::testArgs()
{
	QFETCH( QString, pType );
	QFETCH( QString, pResult );

	LuaTestData		TD;

	Object			*Root = TD.OM.rootObject();

	Root->propAdd( "result", "no result", Root->id() );

	Verb			V;

	V.initialise();

	V.setDirectObjectArgument( NONE );
	V.setIndirectObjectArgument( NONE );
	V.setPrepositionArgument( NONE );

	V.setScript( QString( "moo.root.result = tostring( moo.%1 )" ).arg( pType ) );

	V.setOwner( Root->id() );

	Root->verbAdd( "test", V );

	lua_task		T = TD.execute( QString( "test" ) );

	QCOMPARE( T.error(), false );

	QCOMPARE( Root->propValue( "result" ).toString(), pResult );
}

void cmd_permissions::testArgs2_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<QString>( "pResult" );

	QTest::newRow( "player" ) << "player" << "#3";
	QTest::newRow( "caller" ) << "caller" << "#3";
	QTest::newRow( "object" ) << "object" << "#3";
	QTest::newRow( "permissions" ) << "permissions" << "#1";
	QTest::newRow( "verb" ) << "verb" << "test";
	QTest::newRow( "argstr" ) << "argstr" << "";
	QTest::newRow( "dobjstr" ) << "dobjstr" << "";
	QTest::newRow( "dobj" ) << "dobj" << "#-1";
	QTest::newRow( "iobjstr" ) << "iobjstr" << "";
	QTest::newRow( "iobj" ) << "iobj" << "#-1";
	QTest::newRow( "prepstr" ) << "prepstr" << "";
}

void cmd_permissions::testArgs2()
{
	QFETCH( QString, pType );
	QFETCH( QString, pResult );

	LuaTestData		TD;

	Object			*Root = TD.OM.rootObject();

	Root->propAdd( "result", "no result", Root->id() );

	if( true )
	{
		Verb			V;

		V.initialise();

		V.setDirectObjectArgument( NONE );
		V.setIndirectObjectArgument( NONE );
		V.setPrepositionArgument( NONE );

		V.setScript( QString( "moo.root.result = tostring( moo.%1 )" ).arg( pType ) );

		V.setOwner( Root->id() );

		Root->verbAdd( "test", V );
	}

	if( true )
	{
		Verb			V;

		V.initialise();

		V.setDirectObjectArgument( NONE );
		V.setIndirectObjectArgument( NONE );
		V.setPrepositionArgument( NONE );

		V.setScript( QString( "moo.pass( ... )" ) );

		V.setOwner( TD.programmerId() );

		TD.Programmer->verbAdd( "test", V );
	}

	lua_task		T = TD.execute( QString( "test" ) );

	QCOMPARE( T.error(), false );

	QCOMPARE( Root->propValue( "result" ).toString(), pResult );
}

void cmd_permissions::testArgs3_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<QString>( "pResult" );

	QTest::newRow( "player" ) << "player" << "#3";
	QTest::newRow( "caller" ) << "caller" << "#2";
	QTest::newRow( "object" ) << "object" << "#1";
	QTest::newRow( "permissions" ) << "permissions" << "#1";
	QTest::newRow( "verb" ) << "verb" << "test2";
	QTest::newRow( "argstr" ) << "argstr" << "";
	QTest::newRow( "dobjstr" ) << "dobjstr" << "";
	QTest::newRow( "dobj" ) << "dobj" << "#-1";
	QTest::newRow( "iobjstr" ) << "iobjstr" << "";
	QTest::newRow( "iobj" ) << "iobj" << "#-1";
	QTest::newRow( "prepstr" ) << "prepstr" << "";
}

void cmd_permissions::testArgs3()
{
	QFETCH( QString, pType );
	QFETCH( QString, pResult );

	LuaTestData		TD;

	TD.Programmer->setWizard( false );

	Object			*Root = TD.OM.rootObject();

	Root->propAdd( "result", "no result", Root->id() );

	Object			*O1 = TD.OM.object( 2 );

	if( true )
	{
		Verb			V;

		V.initialise();

		V.setDirectObjectArgument( NONE );
		V.setIndirectObjectArgument( NONE );
		V.setPrepositionArgument( NONE );

		V.setScript( QString( "moo.root.result = tostring( moo.%1 )" ).arg( pType ) );

		V.setOwner( Root->id() );

		Root->verbAdd( "test2", V );
	}

	if( true )
	{
		Verb			V;

		V.initialise();

		V.setDirectObjectArgument( NONE );
		V.setIndirectObjectArgument( NONE );
		V.setPrepositionArgument( NONE );

		V.setScript( QString( "moo.root:test2()" ) );

		V.setOwner( O1->id() );

		O1->verbAdd( "test", V );
	}

	lua_task		T = TD.execute( QString( "test" ) );

	QCOMPARE( T.error(), false );

	QCOMPARE( Root->propValue( "result" ).toString(), pResult );
}

QTEST_GUILESS_MAIN( cmd_permissions )

#include "cmd_permissions.moc"
