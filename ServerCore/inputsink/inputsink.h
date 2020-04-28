#ifndef INPUTSINK_H
#define INPUTSINK_H

#include <QString>

#include "connection.h"

class InputSink
{
public:
	virtual ~InputSink( void ) {}

	virtual bool input( const QString &pData ) = 0;

	virtual bool output( const QString &pData ) = 0;

	virtual bool screenNeedsReset( void ) const = 0;

	virtual Connection::LineMode lineMode( void ) const = 0;
};

#endif // INPUTSINK_H
