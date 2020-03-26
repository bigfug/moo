#ifndef PROPERTY_H
#define PROPERTY_H

#include "mooglobal.h"

#include <QVariant>
#include <QDataStream>

typedef struct PropertyData
{
	PropertyData( void )
		: mObject( OBJECT_NONE ), mParent( OBJECT_NONE ), mOwner( OBJECT_NONE ),
		  mRead( false ), mWrite( false ), mChange( true )
	{

	}

	ObjectId		mObject;		// The object this property is attached to
	QString			mName;			// The name of this property
	ObjectId		mParent;		// The parent object that defined this property
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

	inline ObjectId object( void ) const
	{
		return( mData.mObject );
	}

	inline QString name( void ) const
	{
		return( mData.mName );
	}

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

	void setObject( ObjectId pObject );

	void setName( QString pName );

	void setPermissions( quint16 pPerms );

	void setParent( ObjectId pParent );

	void setOwner( ObjectId pOwner );

	void setValue( const QVariant &pValue );

	void setRead( bool pValue );

	void setWrite( bool pValue );

	void setChange( bool pValue );

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
	void setUpdated( void );

private:
	PropertyData		mData;
};

#endif // PROPERTY_H
