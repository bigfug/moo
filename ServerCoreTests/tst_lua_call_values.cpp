
#include "tst_lua_call_values.h"

#include "object.h"
#include "objectmanager.h"
#include "connection.h"
#include "lua_task.h"
#include "connectionmanager.h"

#include "luatestdata.h"

/*
  COMMAND PARSING

player    an object, the player who typed the command
this      an object, the object on which this verb was found
caller    an object, the same as `player'
verb      a string, the first word of the command
argstr    a string, everything after the first word of the command
args      a list of strings, the words in `argstr'
dobjstr   a string, the direct object string found during parsing
dobj      an object, the direct object value found during matching
prepstr   a string, the prepositional phrase found during parsing
iobjstr   a string, the indirect object string
iobj      an object, the indirect object value
*/

/*
GENERAL

For others, the general meaning of the value is consistent, though the value itself is
different for different situations:

player
	an object, the player who typed the command that started the task that involved
	running this piece of code.
this
	an object, the object on which the currently-running verb was found.
caller
	an object, the object on which the verb that called the currently-running verb was
	found. For the first verb called for a given command, `caller' has the same value
	as `player'.
verb
	a string, the name by which the currently-running verb was identified.
args
	a list, the arguments given to this verb. For the first verb called for a given
	command, this is a list of strings, the words on the command line.

The rest of the so-called "built-in" variables are only really meaningful for the first
verb called for a given command. Their semantics is given in the discussion of command
parsing, above.
*/

/*
Functions are always called by the program for some verb; that program is running with
the permissions of some player, usually the owner of the verb in question (it is not
always the owner, though; wizards can use set_task_pperms() to change the permissions
`on the fly'). In the function descriptions below, we refer to the player whose
permissions are being used as the programmer.

Function: none set_task_perms (obj who)
	Changes the permissions with which the currently-executing verb is running to be
	those of who. If the programmer is neither who nor a wizard, then E_PERM is raised.

	Note: This does not change the owner of the currently-running verb, only the
	permissions of this particular invocation. It is used in verbs owned by wizards
	to make themselves run with lesser (usually non-wizard) permissions.

*/

/*
Function: list eval (str string)
	The MOO-code compiler processes string as if it were to be the program associated
	with some verb and, if no errors are found, that fictional verb is invoked. If the
	programmer is not, in fact, a programmer, then E_PERM is raised. The normal result
	of calling eval() is a two element list. The first element is true if there were
	no compilation errors and false otherwise. The second element is either the result
	returned from the fictional verb (if there were no compilation errors) or a list of
	the compiler's error messages (otherwise).

	When the fictional verb is invoked, the various built-in variables have values as
	shown below:

	player    the same as in the calling verb
	this      #-1
	caller    the same as the initial value of this in the calling verb

	args      {}
	argstr    ""

	verb      ""
	dobjstr   ""
	dobj      #-1
	prepstr   ""
	iobjstr   ""
	iobj      #-1

	The fictional verb runs with the permissions of the programmer and as if its
	`d' permissions bit were on.

	eval("return 3 + 4;")   =>   {1, 7}

*/

void TestLuaCallValues::luaCallValueFirst_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<bool>( "pWizard" );
	QTest::addColumn<ObjectId>( "pResult" );

	QTest::newRow( "object" )      << "object"      << false << OBJECT_NONE;
	QTest::newRow( "caller" )      << "caller"      << false << 3;
	QTest::newRow( "player" )      << "player"      << false << 3;
	QTest::newRow( "permissions" ) << "permissions" << false << 3;

	QTest::newRow( "object (wizard)" )      << "object"      << true  << OBJECT_NONE;
	QTest::newRow( "caller (wizard)" )      << "caller"      << true  << 0;
	QTest::newRow( "player (wizard)" )      << "player"      << true  << 0;
	QTest::newRow( "permissions (wizard)" ) << "permissions" << true  << 0;
}

void TestLuaCallValues::luaCallValueFirst( void )
{
	QFETCH( QString,  pType );
	QFETCH( bool, pWizard );
	QFETCH( ObjectId, pResult );

	LuaTestData			 TD;

	Object				*O  = TD.OM.newObject();

	O->setParent( 2 );
	O->setOwner( TD.programmerId() );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "string" );
	P.setOwner( TD.programmerId() );

	O->propAdd( "result", P );

	// Call test

	TD.execute( QString( ";o( %1 ).result = moo.%2.id" ).arg( O->id() ).arg( pType ), pWizard ? OBJECT_SYSTEM : TD.programmerId() );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( pResult ) );	// moo.eval sets object to -1
}

void TestLuaCallValues::luaCallValueSecond_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<bool>( "pWizard" );
	QTest::addColumn<ObjectId>( "pResult" );

	QTest::newRow( "object" )      << "object"      << false << 4;
	QTest::newRow( "caller" )      << "caller"      << false << -1;
	QTest::newRow( "player" )      << "player"      << false << 3;
	QTest::newRow( "permissions" ) << "permissions" << false << 4;

	QTest::newRow( "object (wizard)" )      << "object"      << true  << 4;
	QTest::newRow( "caller (wizard)" )      << "caller"      << true  << OBJECT_NONE;
	QTest::newRow( "player (wizard)" )      << "player"      << true  << 3;
	QTest::newRow( "permissions (wizard)" ) << "permissions" << true  << 4;
}

void TestLuaCallValues::luaCallValueSecond()
{
	QFETCH( QString,  pType );
	QFETCH( bool, pWizard );
	QFETCH( ObjectId, pResult );

	LuaTestData			 TD;

	TD.Programmer->setWizard( pWizard );

	Object				*O  = TD.OM.newObject();

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "string" );
	P.setOwner( O->id() );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( O->id() );
	V.setScript( QString( "moo.object.result = tostring( moo.%1.id )" ).arg( pType ) );

	O->verbAdd( "test", V );

	// Call test

	TD.execute( QString( ";o( %1 ):test()" ).arg( O->id() ) );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( pResult ) );
}

void TestLuaCallValues::luaCallValueThird_data()
{
	QTest::addColumn<QString>( "pType" );
	QTest::addColumn<bool>( "pWizard" );
	QTest::addColumn<ObjectId>( "pResult" );

	QTest::newRow( "object" )      << "object"      << false << 5;
	QTest::newRow( "caller" )      << "caller"      << false << 4;
	QTest::newRow( "player" )      << "player"      << false << 3;
	QTest::newRow( "permissions" ) << "permissions" << false << 5;

	QTest::newRow( "object (wizard)" )      << "object"      << true  << 5;
	QTest::newRow( "caller (wizard)" )      << "caller"      << true  << 4;
	QTest::newRow( "player (wizard)" )      << "player"      << true  << 3;
	QTest::newRow( "permissions (wizard)" ) << "permissions" << true  << 5;
}

void TestLuaCallValues::luaCallValueThird()
{
	QFETCH( QString,  pType );
	QFETCH( bool, pWizard );
	QFETCH( ObjectId, pResult );

	LuaTestData			 TD;

	TD.Programmer->setWizard( pWizard );

	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();

	O1->setParent( 2 );
	O2->setParent( 1 );

	// Add property to hold result

	Property		P2;

	P2.initialise();

	P2.setValue( "result" );
	P2.setOwner( O2->id() );

	O2->propAdd( "result", P2 );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ):test()" ).arg( O2->id() ) );

	O1->verbAdd( "test", V1);

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "o( %1 ).result = tostring( moo.%2.id )" ).arg( O2->id() ).arg( pType ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.execute( QString( ";o( %1 ):test()" ).arg( O1->id() ) );

	// Check result

	QVariant R = O2->propValue( "result" );

	QCOMPARE( R.toString(), QString( "%1" ).arg( pResult ) );
}

void TestLuaCallValues::luaCallValueInherit()
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object			*Root = TD.OM.rootObject();

	Root->propAdd( "result", "no result", Root->id() );

	Root->prop( "result" )->setChange( false );

	Object			*O1 = TD.OM.object( 2 );

	if( true )
	{
		Verb			V;

		V.initialise();

		V.setDirectObjectArgument( NONE );
		V.setIndirectObjectArgument( NONE );
		V.setPrepositionArgument( NONE );

		V.setScript( QString( "moo.object.result = tostring( %1 )" ).arg( 23 ) );

		V.setOwner( Root->id() );

		Root->verbAdd( "test", V );
	}

	lua_task T  = TD.execute( QString( ";o( %1 ):test()" ).arg( O1->id() ) );

	QCOMPARE( T.error(), false );

	QCOMPARE( O1->propValue( "result" ).toString(), "23" );
}

QTEST_GUILESS_MAIN( TestLuaCallValues )

#include "tst_lua_call_values.moc"
