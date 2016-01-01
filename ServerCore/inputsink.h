#ifndef INPUTSINK_H
#define INPUTSINK_H

#include <QString>

class InputSink
{
public:
	virtual ~InputSink( void ) {}

	virtual bool input( const QString &pData ) = 0;
};

#endif // INPUTSINK_H
