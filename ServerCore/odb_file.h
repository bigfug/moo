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

	virtual ObjectId findPlayer( QString pName ) const Q_DECL_OVERRIDE;

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
	QString			 mFileName;
	ObjectList		 mPlayers;
};

#endif // ODBFILE_H
