#ifndef CHANGE_H
#define CHANGE_H

namespace change {

class Change
{
public:
	virtual ~Change( void ) {}

	virtual void commit( void ) = 0;

	virtual void rollback( void ) = 0;
};

}
#endif // CHANGE_H
