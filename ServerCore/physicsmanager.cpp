#include "physicsmanager.h"
#ifdef USING_PHYSICS
#include <btBulletDynamicsCommon.h>
#include <Serialize/BulletWorldImporter/btBulletWorldImporter.h>
#endif
#include <QFile>

PhysicsManager::PhysicsManager( QObject *parent ) :
#ifdef USING_PHYSICS
	QObject( parent ), mBroadphase( 0 ), mCollisionConfiguration( 0 ), mCollisionDispatcher( 0 ), mSolver( 0 ), mWorld( 0 )
#else
	QObject( parent )
#endif
{
#ifdef USING_PHYSICS
	mBroadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	mCollisionConfiguration = new btDefaultCollisionConfiguration();
	mCollisionDispatcher = new btCollisionDispatcher( mCollisionConfiguration );

	// The actual physics solver
	mSolver = new btSequentialImpulseConstraintSolver();

	// The world.
	mWorld = new btDiscreteDynamicsWorld( mCollisionDispatcher, mBroadphase, mSolver, mCollisionConfiguration );

	mWorld->setGravity( btVector3( 0, -10, 0 ) );

	btBulletWorldImporter	Imp( mWorld );

	if( Imp.loadFile( "C:\\Users\\Alex\\Dropbox\\moo.bullet" ) )
	{

	}
#endif
}

PhysicsManager::~PhysicsManager( void )
{
#ifdef USING_PHYSICS
	if( mWorld != 0 )
	{
		const static int		 MAX_SER = 1024*1024*5;
		btDefaultSerializer		*Ser = new btDefaultSerializer( MAX_SER );

		if( Ser != 0 )
		{
			mWorld->serialize( Ser );

			QFile		File( "C:\\Users\\Alex\\Dropbox\\moo.bullet" );

			if( File.open( QIODevice::WriteOnly ) )
			{
				File.write( reinterpret_cast<const char *>( Ser->getBufferPointer() ), Ser->getCurrentBufferSize() );
			}

			delete Ser;
		}

		delete mWorld;

		mWorld = 0;
	}

	if( mSolver != 0 )
	{
		delete mSolver;

		mSolver = 0;
	}

	if( mCollisionDispatcher != 0 )
	{
		delete mCollisionDispatcher;

		mCollisionDispatcher = 0;
	}

	if( mCollisionConfiguration != 0 )
	{
		delete mCollisionConfiguration;

		mCollisionConfiguration = 0;
	}

	if( mBroadphase != 0 )
	{
		delete mBroadphase;

		mBroadphase = 0;
	}
#endif
}

