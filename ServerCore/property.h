#ifndef PROPERTY_H
#define PROPERTY_H

#include "mooglobal.h"

#include <QVariant>
#include <QDataStream>

typedef struct PropertyData
{
	ObjectId		mParent;
	ObjectId		mOwner;			// The object that owns this property
	bool			mRead;			// Read permission lets non-owners get the value of the property
	bool			mWrite;			// write permission lets them set that value
	bool			mChange;		// change ownership in descendants
	QVariant		mValue;
} PropertyData;

class Property
{
	friend class ODB;

public:
	enum Permissions
	{
		READ   = ( 1 << 0 ),
		WRITE  = ( 1 << 1 ),
		CHANGE = ( 1 << 2 )
	};

	quint16 permissions( void ) const;

	inline ObjectId parent( void ) const
	{
		return( mData.mParent );
	}

	inline ObjectId owner( void ) const
	{
		return( mData.mOwner );
	}

	inline bool read( void ) const
	{
		return( mData.mRead );
	}

	inline bool write( void ) const
	{
		return( mData.mWrite );
	}

	inline bool change( void ) const
	{
		return( mData.mChange );
	}

	inline const QVariant &value( void ) const
	{
		return( mData.mValue );
	}

	inline QVariant::Type type( void ) const
	{
		return( mData.mValue.type() );
	}

	void setPermissions( quint16 pPerms );

	inline void setParent( ObjectId pParent )
	{
		mData.mParent = pParent;
	}

	inline void setOwner( ObjectId pOwner )
	{
		mData.mOwner = pOwner;
	}

	inline void setValue( const QVariant &pValue )
	{
		mData.mValue = pValue;
	}

	inline void setRead( bool pValue )
	{
		mData.mRead = pValue;
	}

	inline void setWrite( bool pValue )
	{
		mData.mWrite = pValue;
	}

	inline void setChange( bool pValue )
	{
		mData.mChange = pValue;
	}

	void initialise( void );

protected:
	PropertyData &data( void )
	{
		return( mData );
	}

	const PropertyData &data( void ) const
	{
		return( mData );
	}

private:
	PropertyData		mData;
};

#endif // PROPERTY_H
