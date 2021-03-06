#ifndef ODBFILE_H
#define ODBFILE_H

#include <QString>

#include "odb.h"
#include "objectmanager.h"
#include "taskentry.h"

class Object;
class Verb;
class Property;

class ODBFile : public ODB
{
	Q_OBJECT

public:
	ODBFile( const QString &pFileName = "" );

	virtual ~ODBFile( void ) {}

	virtual void load( void ) Q_DECL_OVERRIDE;
	virtual void save( void ) Q_DECL_OVERRIDE;

	virtual Object *object( ObjectId pIndex ) const Q_DECL_OVERRIDE
	{
		Q_UNUSED( pIndex )

		return( 0 );
	}

	virtual bool hasObject( ObjectId pIndex ) const Q_DECL_OVERRIDE
	{
		Q_UNUSED( pIndex )

		return( false );
	}

	virtual void addObject( Object &pObject ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
	}

	virtual void deleteObject( Object &pObject ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
	}

	virtual void updateObject( Object &pObject ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
	}

	virtual void addVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void deleteVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void updateVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void addProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void deleteProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void updateProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE
	{
		Q_UNUSED( pObject )
		Q_UNUSED( pName )
	}

	virtual void checkpoint() Q_DECL_OVERRIDE;

	virtual ObjectId findPlayer( QString pName ) const Q_DECL_OVERRIDE;
	virtual ObjectId findByProp( QString pName, const QVariant &pValue ) const Q_DECL_OVERRIDE;

	virtual void addSignalConnection( const SignalConnection &SC ) Q_DECL_OVERRIDE
	{

	}

	virtual void deleteSignalConnection( const SignalConnection &SC ) Q_DECL_OVERRIDE
	{

	}

	virtual QVector<SignalConnection> signalConnections( ObjectId pSrcObj ) const Q_DECL_OVERRIDE
	{
		return( QVector<SignalConnection>() );
	}

	virtual void addTask(TaskEntry &TE) Q_DECL_OVERRIDE
	{
		mTaskQueue << TE;

		std::sort( mTaskQueue.begin(), mTaskQueue.end(), TaskEntry::lessThan );
	}

	virtual QList<TaskEntry> tasks(qint64 pTimeStamp) Q_DECL_OVERRIDE
	{
		QList<TaskEntry>		TskLst;

		while( !mTaskQueue.isEmpty() && mTaskQueue.first().timestamp() <= pTimeStamp )
		{
			TskLst << mTaskQueue.takeFirst();
		}

		return( TskLst );
	}

	virtual qint64 nextTaskTime( void ) Q_DECL_OVERRIDE
	{
		return( mTaskQueue.isEmpty() ? -1 : mTaskQueue.first().timestamp() );
	}

	virtual void killTask( TaskId pTaskId ) Q_DECL_OVERRIDE
	{
		for( QList<TaskEntry>::iterator it = mTaskQueue.begin() ; it != mTaskQueue.end() ; it++ )
		{
			if( it->id() == pTaskId )
			{
				mTaskQueue.erase( it );

				break;
			}
		}
	}

	virtual ObjectIdVector children( ObjectId pParentId ) const Q_DECL_OVERRIDE
	{
		return( ObjectIdVector() );
	}

	virtual int childrenCount( ObjectId pParentId ) const Q_DECL_OVERRIDE
	{
		return( -1 );
	}

	virtual QMap<ObjectId,QString> objectNames( ObjectIdVector pIds ) const Q_DECL_OVERRIDE
	{
		return( QMap<ObjectId,QString>() );
	}

	virtual QString objectName( ObjectId pId ) const Q_DECL_OVERRIDE
	{
		return( QString() );
	}

	virtual ObjectId objectParent( ObjectId pId ) const Q_DECL_OVERRIDE
	{
		return( OBJECT_NONE );
	}

	virtual void exportModule( ObjectId pModuleId, const QString &pFileName, TransferInformation &pTrnInf ) const Q_DECL_OVERRIDE
	{

	}

	virtual ObjectId importModule( ObjectId pParentId, ObjectId pOwnerId, const QString &pFileName, TransferInformation &pTrnInf ) Q_DECL_OVERRIDE
	{
		return( OBJECT_NONE );
	}

	virtual ObjectIdVector connectedObjects( void ) const Q_DECL_OVERRIDE
	{
		return( ObjectIdVector() );
	}

private:
	void loadObject( QDataStream &DS, Object &O );
	void updateObject( QDataStream &DS, const Object &O );

	void loadVerb( QDataStream &DS, Verb &V );
	void saveVerb( QDataStream &DS, const Verb &V );

	void loadProperty( QDataStream &DS, Property &P );
	void saveProperty( QDataStream &DS, const Property &P );

	void loadTask( QDataStream &DS, TaskEntry &TE );
	void saveTask( QDataStream &DS, const TaskEntry &TE );

private:
	QString					 mFileName;
	ObjectList				 mPlayers;
	QList<TaskEntry>		 mTaskQueue;
};

#endif // ODBFILE_H
