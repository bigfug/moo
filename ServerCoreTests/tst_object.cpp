
#include "tst_object.h"

#include "object.h"
#include "objectmanager.h"

void TestObject::objectDefaults( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O;
	QList<ObjectId>	 idList;

	// create an object

	O = OM.newObject();

	QVERIFY( O != 0 );

	O = OM.object( 0 );

	QVERIFY( O != 0 );

	QCOMPARE( O->id(), 0 );

	// check defaults

	idList.clear();

	O->ancestors( idList );

	QVERIFY( idList.empty() );

	idList.clear();

	O->descendants( idList );

	QVERIFY( idList.empty() );

	QVERIFY( O->children().empty() );

	QCOMPARE( O->id(), 0 );
	QCOMPARE( O->location(), -1 );
	QCOMPARE( O->owner(), -1 );
	QCOMPARE( O->parent(), -1 );

	QCOMPARE( O->player(), false );
	QCOMPARE( O->programmer(), false );
	QCOMPARE( O->recycle(), false );
	QCOMPARE( O->valid(), true );
	QCOMPARE( O->wizard(), false );
	QCOMPARE( O->fertile(), false );

	// recycle

	OM.recycle( O );

	QCOMPARE( O->recycle(), true );
	QCOMPARE( O->valid(), false );

	QCOMPARE( OM.object( 0 ), nullptr );
	QCOMPARE( OM.objectIncludingRecycled( 0 ), O );

	OM.recycleObjects();

	QCOMPARE( OM.object( 0 ), nullptr );

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

void TestObject::objectGetSet( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O  = OM.newObject();
	Object			*P  = OM.newObject();
	QString			 TestName = "TestName";
	ObjectId		 idOwner = 129837;

	QVERIFY( O != 0 );
	QVERIFY( P != 0 );

	O->setFertile( true );
	O->setName( TestName );
	O->setOwner( idOwner );
	O->setParent( P->id() );
	O->setPlayer( true );
	O->setProgrammer( true );
	O->setRead( true );
	O->setRecycled( true );
	O->setWizard( true );
	O->setWrite( true );

	QCOMPARE( O->fertile(), true );
	QCOMPARE( O->name(), TestName );
	QCOMPARE( O->owner(), idOwner );
	QCOMPARE( O->parent(), P->id() );
	QCOMPARE( O->player(), true );
	QCOMPARE( O->programmer(), true );
	QCOMPARE( O->read(), true );
	QCOMPARE( O->recycle(), true );
	QCOMPARE( O->write(), true );

	QVERIFY( O->permissions() == ( Object::READ | Object::WRITE | Object::FERTILE ) );

	O->setPermissions( 0 );

	QVERIFY( O->permissions() == 0 );

	O->setParent( -1 );

	OM.recycle( O );
	OM.recycle( P );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

void TestObject::objectTree( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();

	QVERIFY( O1 != 0 );
	QVERIFY( O2 != 0 );
	QVERIFY( O3 != 0 );

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );

	QCOMPARE( O2->parent(), O1->id() );
	QCOMPARE( O3->parent(), O2->id() );

	QVERIFY( O1->children().size() == 1 );

	QList<ObjectId>		ids;

	O1->descendants( ids );

	QVERIFY( ids.size() == 2 );
	QVERIFY( ids[ 0 ] == O2->id() );
	QVERIFY( ids[ 1 ] == O3->id() );

	ids.clear();

	O3->ancestors( ids );

	QVERIFY( ids.size() == 2 );
	QVERIFY( ids[ 0 ] == O2->id() );
	QVERIFY( ids[ 1 ] == O1->id() );

	ids.clear();

	O2->setParent( -1 );
	O3->setParent( -1 );

	QCOMPARE( O2->parent(), -1 );
	QCOMPARE( O3->parent(), -1 );

	OM.recycle( O1 );
	OM.recycle( O2 );
	OM.recycle( O3 );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

void TestObject::objectLocation( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();

	O2->move( O1 );
	O3->move( O1 );

	QVERIFY( O2->location() == O1->id() );
	QVERIFY( O3->location() == O1->id() );

	QVERIFY( O1->contents().size() == 2 );

	QVERIFY( O1->contents().at( 0 ) == O2->id() );
	QVERIFY( O1->contents().at( 1 ) == O3->id() );

	O3->move( O2 );

	QVERIFY( O2->location() == O1->id() );
	QVERIFY( O3->location() == O2->id() );

	QVERIFY( O1->contents().size() == 1 );
	QVERIFY( O2->contents().size() == 1 );

	QVERIFY( O1->contents().at( 0 ) == O2->id() );
	QVERIFY( O2->contents().at( 0 ) == O3->id() );

	O2->move( 0 );

	QVERIFY( O2->location() == -1 );

	QVERIFY( O1->contents().size() == 0 );

	O3->move( 0 );

	QVERIFY( O3->location() == -1 );

	QVERIFY( O2->contents().size() == 0 );

	OM.recycle( O1 );
	OM.recycle( O2 );
	OM.recycle( O3 );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

/*
  1
   \
	2
   / \
  5   3
	   \
		4
*/

void TestObject::objectVerbs( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();
	Object			*O4 = OM.newObject();
	Object			*O5 = OM.newObject();
	Verb			 v;

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );
	O4->setParent( O3->id() );
	O5->setParent( O2->id() );

	v.initialise();

	O1->verbAdd( "v1", v );

	QVERIFY( O1->verbs().size() == 1 );

	O1->verbAdd( "d1", v );
	O1->verbAdd( "d2", v );
	O2->verbAdd( "d3", v );
	O2->verbAdd( "d4", v );
	O3->verbAdd( "d5", v );
	O3->verbAdd( "d6", v );
	O4->verbAdd( "d7", v );
	O4->verbAdd( "d8", v );
	O5->verbAdd( "d9", v );
	O5->verbAdd( "d0", v );

	QVERIFY( O1->verbs().size() == 3 );

	QVERIFY( O1->verbMatch( "v1" ) != 0 );
	QVERIFY( O2->verbMatch( "v1" ) == 0 );
	QVERIFY( O3->verbMatch( "v1" ) == 0 );
	QVERIFY( O4->verbMatch( "v1" ) == 0 );
	QVERIFY( O5->verbMatch( "v1" ) == 0 );

	QVERIFY( O1->verbParent( "v1" ) == 0 );
	QVERIFY( O2->verbParent( "v1" ) != 0 );
	QVERIFY( O3->verbParent( "v1" ) != 0 );
	QVERIFY( O4->verbParent( "v1" ) != 0 );
	QVERIFY( O5->verbParent( "v1" ) != 0 );

	O3->verbAdd( "v2", v );

	QVERIFY( O3->verbs().size() == 3 );

	QVERIFY( O1->verbMatch( "v2" ) == 0 );
	QVERIFY( O2->verbMatch( "v2" ) == 0 );
	QVERIFY( O3->verbMatch( "v2" ) != 0 );
	QVERIFY( O4->verbMatch( "v2" ) == 0 );
	QVERIFY( O5->verbMatch( "v2" ) == 0 );

	QVERIFY( O1->verbParent( "v2" ) == 0 );
	QVERIFY( O2->verbParent( "v2" ) == 0 );
	QVERIFY( O3->verbParent( "v2" ) == 0 );
	QVERIFY( O4->verbParent( "v2" ) != 0 );
	QVERIFY( O5->verbParent( "v2" ) == 0 );

	O1->verbDelete( "v1" );

	QVERIFY( O1->verbMatch( "v1" ) == 0 );

	O3->verbDelete( "v2" );

	QVERIFY( O3->verbMatch( "v2" ) == 0 );

	QVERIFY( O1->verbs().size() == 2 );
	QVERIFY( O3->verbs().size() == 2 );

	O5->setParent( -1 );
	O4->setParent( -1 );
	O3->setParent( -1 );
	O2->setParent( -1 );

	OM.recycle( O1 );
	OM.recycle( O2 );
	OM.recycle( O3 );
	OM.recycle( O4 );
	OM.recycle( O5 );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

void TestObject::objectProps( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();
	Object			*O4 = OM.newObject();
	Object			*O5 = OM.newObject();
	Property		 p;

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );
	O4->setParent( O3->id() );
	O5->setParent( O2->id() );

	QVERIFY( O1->prop( "p1" ) == 0 );

	p.initialise();

	p.setValue( "123" );

	O1->propAdd( "p1", p );

	QVERIFY( O1->prop( "p1" ) != 0 );
	QVERIFY( O1->prop( "p1" )->value() == "123" );

	p.initialise();
	p.setValue( 234 );

	O1->propAdd( "p2", p );

	QVERIFY( O1->prop( "p2" ) != 0 );
	QVERIFY( O1->prop( "p2" )->value() == 234 );

	QVERIFY( O2->prop( "p1" ) == 0 && O2->prop( "p2" ) == 0 );
	QVERIFY( O3->prop( "p1" ) == 0 && O3->prop( "p2" ) == 0 );
	QVERIFY( O4->prop( "p1" ) == 0 && O4->prop( "p2" ) == 0 );
	QVERIFY( O5->prop( "p1" ) == 0 && O5->prop( "p2" ) == 0 );

	QVERIFY( O2->propParent( "p1" ) != 0 && O2->propParent( "p2" ) != 0 );
	QVERIFY( O3->propParent( "p1" ) != 0 && O3->propParent( "p2" ) != 0 );
	QVERIFY( O4->propParent( "p1" ) != 0 && O4->propParent( "p2" ) != 0 );
	QVERIFY( O5->propParent( "p1" ) != 0 && O5->propParent( "p2" ) != 0 );

	O1->propDelete( "p1" );

	QVERIFY( O1->prop( "p1" ) == 0 );
	QVERIFY( O1->properties().size() == 1 );

	O1->propDelete( "p2" );

	QVERIFY( O1->prop( "p2" ) == 0 );
	QVERIFY( O1->properties().size() == 0 );

	O5->setParent( -1 );
	O4->setParent( -1 );
	O3->setParent( -1 );
	O2->setParent( -1 );

	OM.recycle( O1 );
	OM.recycle( O2 );
	OM.recycle( O3 );
	OM.recycle( O4 );
	OM.recycle( O5 );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

void TestObject::objectPropsInherit( void )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*O1 = OM.newObject();
	Object			*O2 = OM.newObject();
	Object			*O3 = OM.newObject();
	Object			*O4 = OM.newObject();
	Object			*O5 = OM.newObject();
	Property		 p;

	O2->setParent( O1->id() );
	O3->setParent( O2->id() );
	O4->setParent( O3->id() );
	O5->setParent( O2->id() );

	p.initialise();
	p.setValue( "p1" );
	p.setPermissions( Property::READ | Property::WRITE );

	O1->propAdd( "p1", p );

	p.setValue( "p1-2" );

	O3->propSet( "p1", "p1-2" );

	QVERIFY( O1->prop( "p1" ) != 0 );
	QVERIFY( O2->prop( "p1" ) == 0 );
	QVERIFY( O3->prop( "p1" ) != 0 );
	QVERIFY( O4->prop( "p1" ) == 0 );
	QVERIFY( O5->prop( "p1" ) == 0 );

	QVERIFY( O1->propParent( "p1" ) == 0 );
	QVERIFY( O2->propParent( "p1" ) == O1->prop( "p1" ) );
	QVERIFY( O3->propParent( "p1" ) == O1->prop( "p1" ) );
	QVERIFY( O4->propParent( "p1" ) == O3->prop( "p1" ) );
	QVERIFY( O5->propParent( "p1" ) == O1->prop( "p1" ) );

	Property	*FndPrp;
	Object		*FndObj;

	QVERIFY( O1->propFind( "p1", &FndPrp, &FndObj ) == true );
	QCOMPARE( FndPrp, O1->prop( "p1" ) );
	QCOMPARE( FndObj, O1 );

	QVERIFY( O2->propFind( "p1", &FndPrp, &FndObj ) == true );
	QCOMPARE( FndPrp, O1->prop( "p1" ) );
	QCOMPARE( FndObj, O1 );

	QVERIFY( O3->propFind( "p1", &FndPrp, &FndObj ) == true );
	QCOMPARE( FndPrp, O3->prop( "p1" ) );
	QCOMPARE( FndObj, O3 );

	QVERIFY( O4->propFind( "p1", &FndPrp, &FndObj ) == true );
	QCOMPARE( FndPrp, O3->prop( "p1" ) );
	QCOMPARE( FndObj, O3 );

	QVERIFY( O5->propFind( "p1", &FndPrp, &FndObj ) == true );
	QCOMPARE( FndPrp, O1->prop( "p1" ) );
	QCOMPARE( FndObj, O1 );

	O5->setParent( -1 );
	O4->setParent( -1 );
	O3->setParent( -1 );
	O2->setParent( -1 );

	OM.recycle( O1 );
	OM.recycle( O2 );
	OM.recycle( O3 );
	OM.recycle( O4 );
	OM.recycle( O5 );

	OM.recycleObjects();

	QVERIFY( OM.objectCount() == 0 );

	ObjectManager::reset();
}

QTEST_GUILESS_MAIN( TestObject )

#include "tst_object.moc"
