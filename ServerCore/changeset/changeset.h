#ifndef CHANGESET_H
#define CHANGESET_H

#include <QVector>

#include "change.h"

namespace change {

class ChangeSet
{
public:
	ChangeSet();

	virtual ~ChangeSet( void );

	inline void add( Change *pChange )
	{
		mChanges << pChange;
	}

	void rollback( void );

	void commit( void );

private:
	QVector<Change *>		 mChanges;
};

}

#endif // CHANGESET_H
