#ifndef PROPERTY_H
#define PROPERTY_H

#include "mooglobal.h"

#include <QVariant>
#include <QDataStream>

class Property
{
public:
	enum Permissions
	{
		READ   = ( 1 << 0 ),
		WRITE  = ( 1 << 1 ),
		CHANGE = ( 1 << 2 )
	};

	void save( QDataStream &pData ) const;
	void load( QDataStream &pData );

	quint16 permissions( void ) const;

	inline ObjectId parent( void ) const
	{
		return( mParent );
	}

	inline ObjectId owner( void ) const
	{
		return( mOwner );
	}

	inline bool read( void ) const
	{
		return( mRead );
	}

	inline bool write( void ) const
	{
		return( mWrite );
	}

	inline bool change( void ) const
	{
		return( mChange );
	}

	inline const QVariant &value( void ) const
	{
		return( mValue );
	}

	inline QVariant::Type type( void ) const
	{
		return( mValue.type() );
	}

	void setPermissions( quint16 pPerms );

	inline void setParent( ObjectId pParent )
	{
		mParent = pParent;
	}

	inline void setOwner( ObjectId pOwner )
	{
		mOwner = pOwner;
	}

	inline void setValue( const QVariant &pValue )
	{
		mValue = pValue;
	}

	inline void setRead( bool pValue )
	{
		mRead = pValue;
	}

	inline void setWrite( bool pValue )
	{
		mWrite = pValue;
	}

	inline void setChange( bool pValue )
	{
		mChange = pValue;
	}

	void initialise( void );

private:
	ObjectId		mParent;
	ObjectId		mOwner;			// The object that owns this property
	bool			mRead;			// Read permission lets non-owners get the value of the property
	bool			mWrite;			// write permission lets them set that value
	bool			mChange;		// change ownership in descendants
	QVariant		mValue;
};

#endif // PROPERTY_H
