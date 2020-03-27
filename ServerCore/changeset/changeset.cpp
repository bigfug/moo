#include "changeset.h"

#include <iterator>

using namespace change;

ChangeSet::ChangeSet()
{

}

ChangeSet::ChangeSet( ChangeSet &&c )
	: mChanges( std::move( c.mChanges ) )
{

}

ChangeSet::~ChangeSet()
{
	Q_ASSERT( mChanges.empty() );
}

void ChangeSet::rollback()
{
	while( !mChanges.isEmpty() )
	{
		Change *C = mChanges.takeLast();

		C->rollback();

		delete C;
	}
}

void ChangeSet::commit()
{
	while( !mChanges.isEmpty() )
	{
		Change *C = mChanges.takeFirst();

		C->commit();

		delete C;
	}
}
