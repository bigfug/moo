
#include "tst_servertest.h"

#include "object.h"

void ServerTest::verbGetSet( void )
{
	Verb			V;

	V.initialise();

	V.setPermissions( Verb::READ );

	QVERIFY( V.permissions() == Verb::READ );

	V.setPermissions( Verb::WRITE );

	QVERIFY( V.permissions() == Verb::WRITE );

	V.setPermissions( Verb::EXECUTE );

	QVERIFY( V.permissions() == Verb::EXECUTE );

	V.setObject( 123123 );	QVERIFY( V.object() == 123123 );
	V.setOwner( 234234 );	QVERIFY( V.owner() == 234234 );

	V.setRead( true );		QCOMPARE( V.read(), true );
	V.setRead( false );		QCOMPARE( V.read(), false );

	V.setWrite( true );		QCOMPARE( V.write(), true );
	V.setWrite( false );	QCOMPARE( V.write(), false );

	V.setExecute( true );	QCOMPARE( V.execute(), true );
	V.setExecute( false );	QCOMPARE( V.execute(), false );

	QVERIFY( V.dirty() == false );

	V.setScript( "asdasd" );	QCOMPARE( V.script(), QString( "asdasd" ) );

	QVERIFY( V.dirty() == true );

	QCOMPARE( V.compile(), 3 );

	QVERIFY( V.dirty() == true );

	V.setScript( "print( \"Hello, World!\\n\" );" );

	QVERIFY( V.compile() == 0 );

	QVERIFY( V.dirty() == false );

	QVERIFY( V.directObject() == Verb::ANY );
	QVERIFY( V.indirectObject() == Verb::ANY );
	QVERIFY( V.prepositionType() == Verb::ANY );
	QVERIFY( V.preposition().isEmpty() );

	V.setDirectObjectArgument( Verb::NONE );
	QVERIFY( V.directObject() == Verb::NONE );

	V.setDirectObjectArgument( Verb::THIS );
	QVERIFY( V.directObject() == Verb::THIS );

	V.setDirectObjectArgument( Verb::ANY );
	QVERIFY( V.directObject() == Verb::ANY );

	V.setIndirectObjectArgument( Verb::NONE );
	QVERIFY( V.indirectObject() == Verb::NONE );

	V.setIndirectObjectArgument( Verb::THIS );
	QVERIFY( V.indirectObject() == Verb::THIS );

	V.setIndirectObjectArgument( Verb::ANY );
	QVERIFY( V.indirectObject() == Verb::ANY );

	V.setPrepositionArgument( Verb::NONE );
	QVERIFY( V.prepositionType() == Verb::NONE );
	QVERIFY( V.preposition().isEmpty() );

	V.setPrepositionArgument( "from" );
	QVERIFY( V.prepositionType() == Verb::THIS );
	QCOMPARE( V.preposition(), QString( "from" ) );

	V.setPrepositionArgument( Verb::ANY );
	QVERIFY( V.prepositionType() == Verb::ANY );
	QVERIFY( V.preposition().isEmpty() );
}

void ServerTest::verbArgs( void )
{
	Verb			V;

	V.initialise();

	V.setDirectObjectArgument( Verb::NONE );
	V.setIndirectObjectArgument( Verb::NONE );
	V.setPrepositionArgument( Verb::NONE );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );

	// Test Direct Object

	V.setDirectObjectArgument( Verb::ANY );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, 123, "", OBJECT_NONE ) );

	V.setDirectObjectArgument( Verb::THIS );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( !V.matchArgs( 123, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( !V.matchArgs( OBJECT_NONE, 123, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( 123, 123, "", OBJECT_NONE ) );

	V.setDirectObjectArgument( Verb::NONE );

	// Test Indirect Object

	V.setIndirectObjectArgument( Verb::ANY );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", 123 ) );

	V.setIndirectObjectArgument( Verb::THIS );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( !V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", 123 ) );
	QVERIFY( !V.matchArgs( 123, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( 123, OBJECT_NONE, "", 123 ) );

	V.setIndirectObjectArgument( Verb::NONE );

	// Test Preposition

	V.setPrepositionArgument( Verb::NONE );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( !V.matchArgs( OBJECT_NONE, OBJECT_NONE, "burble", OBJECT_NONE ) );

	V.setPrepositionArgument( Verb::ANY );

	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "burble", OBJECT_NONE ) );

	V.setPrepositionArgument( "burble/bumflick/doodah" );

	QVERIFY( !V.matchArgs( OBJECT_NONE, OBJECT_NONE, "", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "burble", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "bumflick", OBJECT_NONE ) );
	QVERIFY( V.matchArgs( OBJECT_NONE, OBJECT_NONE, "doodah", OBJECT_NONE ) );
	QVERIFY( !V.matchArgs( OBJECT_NONE, OBJECT_NONE, "hoorah", OBJECT_NONE ) );
}

void ServerTest::verbParse( void )
{
	QString			Args;
	QStringList		Test = Verb::parse( "foo \"bar mumble\" baz\" \"fr\"otz\" bl\"o\"rt", Args );

	QVERIFY( Test.size() == 4 );

	QVERIFY( Test[ 0 ] == "foo" );
	QVERIFY( Test[ 1 ] == "bar mumble" );
	QVERIFY( Test[ 2 ] == "baz frotz" );
	QVERIFY( Test[ 3 ] == "blort" );

	QCOMPARE( Args, QString( "\"bar mumble\" baz\" \"fr\"otz\" bl\"o\"rt" ) );

	Test = Verb::parse( "\"Hello, World!", Args );

	QCOMPARE( Test.size(), 2 );

	QCOMPARE( Test[ 0 ], QString( "say" ) );
	QCOMPARE( Test[ 1 ], QString( "Hello, World!" ) );

	QVERIFY( Args == "Hello, World!" );

	Test = Verb::parse( ":smiles", Args );

	QVERIFY( Test.size() == 2 );

	QVERIFY( Test[ 0 ] == "emote" );
	QVERIFY( Test[ 1 ] == "smiles" );

	QVERIFY( Args == "smiles" );

	Test = Verb::parse( ";2 + 2", Args );

	QCOMPARE( Test.size(), 2 );

	QVERIFY( Test[ 0 ] == "eval" );
	QVERIFY( Test[ 1 ] == "2 + 2" );

	QCOMPARE( Args, QString( "2 + 2" ) );

	Test = Verb::parse( "This is a \"\\\"QUOTE TEST\\\"\"", Args );

	QVERIFY( Test.size() == 4 );

	QVERIFY( Test[ 0 ] == "This" );
	QVERIFY( Test[ 1 ] == "is" );
	QVERIFY( Test[ 2 ] == "a" );
	QVERIFY( Test[ 3 ] == "\"QUOTE TEST\"" );

	QCOMPARE( Args, QString( "is a \"\\\"QUOTE TEST\\\"\"" ) );

//	Test = Verb::parse( ";$test_function( \"$test2\" );", Args );

//	QCOMPARE( Test[ 0 ], QString( "eval" ) );
//	QCOMPARE( Test[ 1 ], QString( "o(0):test_function( \"$test2\" );" ) );
}

/*
In this case, the verb-name matches only itself; that is, the name must be
matched exactly.

If the name contains a single star, however, then the name matches any prefix
of itself that is at least as long as the part before the star. For example,
the verb-name `foo*bar' matches any of the strings `foo', `foob', `fooba', or
`foobar'; note that the star itself is not considered part of the name.

If the verb name ends in a star, then it matches any string that begins with
the part before the star. For example, the verb-name `foo*' matches any of the
strings `foo', `foobar', `food', or `foogleman', among many others.

As a special case, if the verb-name is `*' (i.e., a single star all by itself),
then it matches anything at all.
*/

void ServerTest::verbMatch( void )
{
	QVERIFY( Verb::matchName( "test_verb", "test_verb" ) == true );
	QVERIFY( Verb::matchName( "test_verb", "test_verb2" ) == false );

	QVERIFY( Verb::matchName( "foo*bar", "fo" ) == false );
	QVERIFY( Verb::matchName( "foo*bar", "foo" ) == true );
	QVERIFY( Verb::matchName( "foo*bar", "foob" ) == true );
	QVERIFY( Verb::matchName( "foo*bar", "fooba" ) == true );
	QVERIFY( Verb::matchName( "foo*bar", "foobar" ) == true );
	QVERIFY( Verb::matchName( "foo*bar", "foobar2" ) == false );

	QVERIFY( Verb::matchName( "foo*", "fo" ) == false );
	QVERIFY( Verb::matchName( "foo*", "foo" ) == true );
	QVERIFY( Verb::matchName( "foo*", "foobar" ) == true );
	QVERIFY( Verb::matchName( "foo*", "food" ) == true );
	QVERIFY( Verb::matchName( "foo*", "foogleman" ) == true );
	QVERIFY( Verb::matchName( "foo*", "no_match" ) == false );

	QVERIFY( Verb::matchName( "*", "fo" ) == true );
	QVERIFY( Verb::matchName( "*", "foo" ) == true );
	QVERIFY( Verb::matchName( "*", "foob" ) == true );
	QVERIFY( Verb::matchName( "*", "fooba" ) == true );
	QVERIFY( Verb::matchName( "*", "foobar" ) == true );
	QVERIFY( Verb::matchName( "*", "foobar2" ) == true );

	QVERIFY( Verb::matchName( "g*et t*ake", "abc" ) == false );

	QVERIFY( Verb::matchName( "g*et t*ake", "g" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "ge" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "get" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "get2" ) == false );

	QVERIFY( Verb::matchName( "g*et t*ake", "t" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "ta" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "tak" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "take" ) == true );
	QVERIFY( Verb::matchName( "g*et t*ake", "take2" ) == false );
}


