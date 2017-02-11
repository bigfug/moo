
#include "tst_servertest.h"

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

void ServerTest::luaCallValueObjectFirst( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.object.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );
	O->setOwner( TD.Programmer->id() );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( OBJECT_NONE ) );	// moo.eval sets object to -1
}

void ServerTest::luaCallValueCallerFirst( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.caller.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );
	O->setOwner( TD.Programmer->id() );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( 2 ) );	 // #2.eval is the defined verb
}

void ServerTest::luaCallValuePlayerFirst( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.player.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );
	O->setOwner( TD.Programmer->id() );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Con->player() ) );
}

void ServerTest::luaCallValueProgrammerFirst( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.programmer.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Programmer->id() ) );
}

//------

void ServerTest::luaCallValueObjectSecond( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O  = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( O->id() );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( O->id() );
	V.setScript( "moo.object.result = tostring( moo.object.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O->id() ) );
}

void ServerTest::luaCallValueCallerSecond( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( 2 );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.caller.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( OBJECT_NONE ) );
}

void ServerTest::luaCallValuePlayerSecond( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( 2 );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.player.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Con->player() ) );
}

void ServerTest::luaCallValueProgrammerSecond( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( 2 );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.programmer.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	// the same value as it had initially in the calling verb or,
	// if the calling verb is running with wizard permissions,
	// the same as the current value in the calling verb.

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( V.owner() ) );
}

//------

void ServerTest::luaCallValueObjectThird( void )
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O2->id() ) );
}

void ServerTest::luaCallValuePlayerThird()
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.player.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Con->player() ) );
}

void ServerTest::luaCallValueCallerThird()
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.caller.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) );
}

void ServerTest::luaCallValueProgrammerThird()
{
	LuaTestData			 TD;

	TD.Programmer->setWizard( false );

	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.programmer.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O2->id() ) );
}

//--------
//--------

void ServerTest::luaCallValueObjectFirstWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.object.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( OBJECT_NONE ) );
}

void ServerTest::luaCallValueCallerFirstWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.caller.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( 2 ) );
}

void ServerTest::luaCallValuePlayerFirstWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.player.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );
	P.setOwner( TD.Programmer->id() );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Con->player() ) );
}

void ServerTest::luaCallValueProgrammerFirstWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ).result = moo.programmer.id" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );

	O->propAdd( "result", P );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Programmer->id() ) );
}

//------

void ServerTest::luaCallValueObjectSecondWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.object.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O->id() ) );
}

void ServerTest::luaCallValueCallerSecondWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.caller.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( OBJECT_NONE ) );
}

void ServerTest::luaCallValuePlayerSecondWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.player.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Con->player() ) );
}

void ServerTest::luaCallValueProgrammerSecondWizard( void )
{
	LuaTestData			 TD;
	Object				*O     = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O->setParent( 2 );

	// Add property to hold result

	Property		P;

	P.initialise();

	P.setValue( "result" );

	O->propAdd( "result", P );

	// Add verb

	Verb			 V;

	V.initialise();

	V.setOwner( 2 );
	V.setScript( "moo.object.result = tostring( moo.programmer.id )" );

	O->verbAdd( "test", V );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Programmer->id() ) );
}

//------

void ServerTest::luaCallValueObjectThirdWizard( void )
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O1->setParent( 2 );
	O2->setParent( 1 );

	// Add property to hold result

	Property		P2;

	P2.initialise();

	P2.setValue( "result" );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.object.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O2->id() ) );
}

void ServerTest::luaCallValuePlayerThirdWizard()
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O1->setParent( 2 );
	O2->setParent( 1 );

	// Add property to hold result

	Property		P2;

	P2.initialise();

	P2.setValue( "result" );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.player.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Programmer->id() ) );
}

void ServerTest::luaCallValueCallerThirdWizard()
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O1->setParent( 2 );
	O2->setParent( 1 );

	// Add property to hold result

	Property		P2;

	P2.initialise();

	P2.setValue( "result" );

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
	V2.setScript( QString( "o( %1 ).result = tostring( moo.caller.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( O1->id() ) );
}

void ServerTest::luaCallValueProgrammerThirdWizard()
{
	LuaTestData			 TD;
	Object				*O1 = TD.OM.newObject();
	Object				*O2 = TD.OM.newObject();
	QString				 CD = QString( ";o( %1 ):test()" ).arg( O1->id() );

	TD.initTask( CD, TD.Programmer->id() );

	O1->setParent( 2 );
	O2->setParent( 1 );

	// Add property to hold result

	Property		P2;

	P2.initialise();

	P2.setValue( "result" );

	O2->propAdd( "result", P2 );

	// Add verb

	Verb			 V1;

	V1.initialise();

	V1.setOwner( O1->id() );
	V1.setScript( QString( "o( %1 ):test()" ).arg( O2->id() ) );

	O1->verbAdd( "test", V1 );

	// Add verb

	Verb			 V2;

	V2.initialise();

	V2.setOwner( O2->id() );
	V2.setScript( QString( "o( %1 ).result = tostring( moo.programmer.id )" ).arg( O2->id() ) );

	O2->verbAdd( "test", V2 );

	// Call test

	TD.Com->execute( TD.TimeStamp );

	// Check result

	Property	*R = O2->prop( "result" );

	QVERIFY( R != 0 );
	QVERIFY( R->type() == QVariant::String );
	QCOMPARE( R->value().toString(), QString( "%1" ).arg( TD.Programmer->id() ) );
}
