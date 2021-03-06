#ifndef INPUTSINKCOMMAND_H
#define INPUTSINKCOMMAND_H

#include "mooglobal.h"
#include "inputsink.h"
#include "lineedit.h"

class Connection;

class InputSinkCommand : public QObject, public InputSink
{
	Q_OBJECT

public:
	InputSinkCommand( Connection *C );

	virtual bool input( const QString &pData ) Q_DECL_OVERRIDE;

	virtual bool output( const QString &pData ) Q_DECL_OVERRIDE;

	virtual bool screenNeedsReset( void ) const Q_DECL_OVERRIDE
	{
		return( false );
	}

	virtual Connection::LineMode lineMode( void ) const Q_DECL_OVERRIDE
	{
		return( Connection::REALTIME );
	}

private:
	Connection		*mConnection;
	LineEdit		 mLineEdit;
	QStringList		 mCommandHistory;
};

#endif // INPUTSINKCOMMAND_H
