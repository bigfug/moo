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
	for( QVector<Change *>::reverse_iterator it = mChanges.rbegin() ; it != mChanges.rend() ; it++ )
	{
		(*it)->rollback();
	}

	qDeleteAll( mChanges );

	mChanges.clear();
}

void ChangeSet::commit()
{
	for( QVector<Change *>::iterator it = mChanges.begin() ; it != mChanges.end() ; it++ )
	{
		(*it)->commit();
	}

	qDeleteAll( mChanges );

	mChanges.clear();
}
