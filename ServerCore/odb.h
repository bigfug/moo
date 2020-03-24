#ifndef ODB_H
#define ODB_H

#include <QObject>

#include "taskentry.h"
#include "object.h"
#include "property.h"
#include "verb.h"
#include "objectmanager.h"

class ODB : public QObject
{
	Q_OBJECT

public:
	ODB( void ) {}

	virtual ~ODB( void ) {}

	virtual void load( void ) = 0;
	virtual void save( void ) = 0;

	virtual ObjectId findPlayer( QString pName ) const = 0;
	virtual ObjectId findByProp( QString pName, const QVariant &pValue ) const = 0;

	virtual Object *object( ObjectId pIndex ) const = 0;

	virtual bool hasObject( ObjectId pIndex ) const = 0;

	virtual void addObject( Object &pObject ) = 0;
	virtual void deleteObject( Object &pObject ) = 0;
	virtual void updateObject( Object &pObject ) = 0;

	virtual void addVerb( Object &pObject, QString pName ) = 0;
	virtual void deleteVerb( Object &pObject, QString pName ) = 0;
	virtual void updateVerb( Object &pObject, QString pName ) = 0;

	virtual void addProperty( Object &pObject, QString pName ) = 0;
	virtual void deleteProperty( Object &pObject, QString pName ) = 0;
	virtual void updateProperty( Object &pObject, QString pName ) = 0;

	virtual void addTask( TaskEntry &TE ) = 0;

	virtual QList<TaskEntry> tasks( qint64 pTimeStamp ) = 0;

	virtual qint64 nextTaskTime( void ) = 0;

	virtual void killTask( TaskId pTaskId ) = 0;

	virtual void checkpoint( void ) = 0;

	virtual int childrenCount( ObjectId pParentId ) const = 0;

	virtual ObjectIdVector children( ObjectId pParentId ) const = 0;

	virtual QMap<ObjectId,QString> objectNames( ObjectIdVector pIds ) const = 0;

	virtual QString objectName( ObjectId pId ) const = 0;

	virtual ObjectId objectParent( ObjectId pId ) const = 0;

protected:
	Object *newObject( void ) const;

	ObjectManagerData &data( ObjectManager &M ) const;

	const ObjectManagerData &data( const ObjectManager &M ) const;

	ObjectData &data( Object &O ) const;

	const ObjectData &data( const Object &O ) const;

	PropertyData &data( Property &P ) const;

	const PropertyData &data( const Property &P ) const;

	FuncData &funcdata( Func &F ) const;

	const FuncData &funcdata( const Func &F ) const;

	VerbData &verbdata( Verb &V ) const;

	const VerbData &verbdata( const Verb &V ) const;

	TaskEntryData &data( TaskEntry &TE ) const;

	const TaskEntryData &data( const TaskEntry &TE ) const;
};

#endif // ODB_H

