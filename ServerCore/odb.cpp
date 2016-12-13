
#include "odb.h"

Object *ODB::newObject() const
{
	return( new Object() );
}

ObjectManagerData &ODB::data(ObjectManager &M) const
{
	return( M.data() );
}

const ObjectManagerData &ODB::data(const ObjectManager &M) const
{
	return( M.data() );
}

ObjectData &ODB::data( Object &O ) const
{
	return( O.data() );
}

const ObjectData &ODB::data( const Object &O ) const
{
	return( O.data() );
}

PropertyData &ODB::data(Property &P) const
{
	return( P.data() );
}

const PropertyData &ODB::data(const Property &P) const
{
	return( P.data() );
}

FuncData &ODB::funcdata(Func &F) const
{
	return( F.data() );
}

const FuncData &ODB::funcdata(const Func &F) const
{
	return( F.data() );
}

VerbData &ODB::verbdata(Verb &V) const
{
	return( V.data() );
}

const VerbData &ODB::verbdata(const Verb &V) const
{
	return( V.data() );
}

TaskEntryData &ODB::data(TaskEntry &TE) const
{
	return( TE.data() );
}

const TaskEntryData &ODB::data( const TaskEntry &TE) const
{
	return( TE.data() );
}
