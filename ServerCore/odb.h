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

	Object *newObject( void ) const;

	ObjectManagerData &data( ObjectManager &M );

	const ObjectManagerData &data( const ObjectManager &M ) const;

	ObjectData &data( Object &O );

	const ObjectData &data( const Object &O ) const;

	PropertyData &data( Property &P );

	const PropertyData &data( const Property &P ) const;

	FuncData &funcdata( Func &F );

	const FuncData &funcdata( const Func &F ) const;

	VerbData &verbdata( Verb &V );

	const VerbData &verbdata( const Verb &V ) const;

	TaskEntryData &data( TaskEntry &TE );

	const TaskEntryData &data( const TaskEntry &TE ) const;
};

#endif // ODB_H

