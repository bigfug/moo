#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#include <QObject>

#ifdef USING_PHYSICS
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
#endif

class PhysicsManager : public QObject
{
	Q_OBJECT

public:
	explicit PhysicsManager( QObject *parent = 0 );
	virtual ~PhysicsManager( void );

signals:
	
public slots:

private:
#ifdef USING_PHYSICS
	btBroadphaseInterface				*mBroadphase;
	btDefaultCollisionConfiguration		*mCollisionConfiguration;
	btCollisionDispatcher				*mCollisionDispatcher;
	btSequentialImpulseConstraintSolver	*mSolver;
	btDiscreteDynamicsWorld				*mWorld;
#endif
};

#endif // PHYSICSMANAGER_H
