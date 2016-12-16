#ifndef MOOGLOBAL_H
#define MOOGLOBAL_H

#include <QtGlobal>

typedef	qint32		ObjectId;
typedef qint32		TaskId;
typedef qint32		ConnectionId;

#define OBJECT_NONE			(-1)
#define OBJECT_AMBIGUOUS	(-2)
#define OBJECT_FAILED_MATCH	(-3)
#define OBJECT_UNSPECIFIED	(-4)

typedef enum mooError
{
	E_NONE = 0,  // No error
	E_TYPE,      // Type mismatch
	E_DIV,       // Division by zero
	E_PERM,      // Permission denied
	E_PROPNF,    // Property not found
	E_VERBNF,    // Verb not found
	E_VARNF,     // Variable not found
	E_INVIND,    // Invalid indirection
	E_RECMOVE,   // Recursive move
	E_MAXREC,    // Too many verb calls
	E_RANGE,     // Range error
	E_ARGS,      // Incorrect number of arguments
	E_NACC,      // Move refused by destination
	E_INVARG,    // Invalid argument
	E_QUOTA,     // Resource limit exceeded
	E_FLOAT,     // Floating-point arithmetic error
	E_MEMORY	 // Out of memory
} mooError;

#endif // MOOGLOBAL_H
