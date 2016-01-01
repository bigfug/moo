
#include "tst_servertest.h"

#include "object.h"
#include "objectmanager.h"

void ServerTest::propGetSet( void )
{
	Property		P;
	ObjectId		idParent = 345;
	ObjectId		idOwner = 987349;
	QVariant		Value = QString( "IUhasd" );

	P.setParent( idParent );
	P.setOwner( idOwner );
	P.setValue( Value );

	QCOMPARE( P.parent(), idParent );
	QCOMPARE( P.owner(), idOwner );
	QCOMPARE( P.value(), Value );

	P.setPermissions( Property::READ );

	QVERIFY( P.permissions() == Property::READ );

	P.setPermissions( Property::WRITE );

	QVERIFY( P.permissions() == Property::WRITE );

	P.setPermissions( Property::CHANGE );

	QVERIFY( P.permissions() == Property::CHANGE );
}

void ServerTest::propInherit( void )
{
	ObjectManager		&OM = *ObjectManager::instance();

	Object				*O[ 5 ];

	for( int i = 0 ; i < 5 ; i++ )
	{
		O[ i ] = OM.newObject();

		if( i > 0 )
		{
			O[ i ]->setParent( O[ i - 1 ]->id() );
		}
	}

	Property			 P;

	P.initialise();

	P.setValue( QVariant( double( 1212.0 ) ) );

	O[ 0 ]->propAdd( "test", P );

	ObjectManager::reset();
}


