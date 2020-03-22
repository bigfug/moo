#include "odb_file.h"

#include <QDateTime>
#include <QDataStream>
#include <QDir>

#include "objectmanager.h"
#include "object.h"

ODBFile::ODBFile( const QString &pFileName )
	: mFileName( pFileName )
{

}

void ODBFile::load()
{
	ObjectManager		&OM = *ObjectManager::instance();
	ObjectManagerData	&Data = data( OM );

	OM.clear();

	QFile		File( mFileName );

	if( !File.open( QIODevice::ReadOnly ) )
	{
		return;
	}

	QDataStream		DS( &File );

	DS >> Data.mObjNum;

	qint32			c;

	DS >> c;

	for( qint32 i = 0 ; i < c ; i++ )
	{
		Object		*O = newObject();

		if( !O )
		{
			return;
		}

		loadObject( DS, *O );

		Data.mObjMap[ O->id() ] = O;

//		if( O->parent() == OBJECT_NONE )
//		{
//			Data.mObjTop.push_back( O );
//		}

		if( O->player() )
		{
			mPlayers.push_back( O );
		}
	}

	// Load Tasks

	QMutexLocker		L( &Data.mTaskMutex );

	qint32			TaskCount;

	DS >> TaskCount;

	for( qint32 i = 0 ; i < TaskCount ; i++ )
	{
		TaskEntry		E;

		loadTask( DS, E );

		Data.mTaskList << E;
	}

	DS >> TaskCount;

	for( qint32 i = 0 ; i < TaskCount ; i++ )
	{
		TaskEntry		E;

		loadTask( DS, E );

		mTaskQueue << E;
	}
}

void ODBFile::save()
{
	ObjectManager			&OM = *ObjectManager::instance();
	const ObjectManagerData	&Data = data( OM );

	QString		DatStr = QDateTime::currentDateTime().toString( "yyyy-MM-dd.hh-mm-ss" );
	QString		DatNam = mFileName.mid( mFileName.lastIndexOf( '\\' ) + 1 );
	QString		NamPrt = DatNam.left( DatNam.lastIndexOf( '.' ) );
	QString		NamExt = DatNam.mid( DatNam.lastIndexOf( '.' ) + 1 );
	QString		NewNam = QString( "%1.%2.tmp" ).arg( NamPrt ).arg( DatStr );
	QString		OldNam = QString( "%1.%2.%3" ).arg( NamPrt ).arg( DatStr ).arg( NamExt );

	QFile		File( NewNam );

	if( !File.open( QIODevice::WriteOnly ) )
	{
		return;
	}

	QDataStream		DS( &File );

	DS << Data.mObjNum;

	qint32			c = Data.mObjMap.size();

	DS << c;

	for( const Object *O : Data.mObjMap.values() )
	{
		updateObject( DS, *O );
	}

	// Save Tasks

	qint32			ListCount = Data.mTaskList.size();

	DS << ListCount;

	for( const TaskEntry &TE : Data.mTaskList )
	{
		saveTask( DS, TE );
	}

	qint32			QueueCount = mTaskQueue.size();

	DS << QueueCount;

	for( const TaskEntry &TE : mTaskQueue )
	{
		saveTask( DS, TE );
	}

	File.close();

	QDir().rename( mFileName, OldNam );
	QDir().rename( NewNam, mFileName );
}

ObjectId ODBFile::findPlayer( QString pName ) const
{
	for( Object *O : mPlayers )
	{
		if( O->name().compare( pName, Qt::CaseSensitive ) )
		{
			return( O->id() );
		}
	}

	return( OBJECT_NONE );
}

ObjectId ODBFile::findByProp(QString pName, const QVariant &pValue) const
{
	return( OBJECT_NONE );
}

void ODBFile::loadObject( QDataStream &DS, Object &O )
{
	ObjectData			&Data = data( O );

	ObjectId			id;
	quint32				c;

	DS >> Data.mId;
	DS >> Data.mPlayer;
	DS >> Data.mParent;

	DS >> c;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		DS >> id;

		Data.mChildren.push_back( id );
	}

	DS >> Data.mName;
	DS >> Data.mOwner;
	DS >> Data.mLocation;

	DS >> c;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		DS >> id;

		Data.mContents.push_back( id );
	}

	DS >> Data.mProgrammer;
	DS >> Data.mWizard;
	DS >> Data.mRead;
	DS >> Data.mWrite;
	DS >> Data.mFertile;

	DS >> c;

	QString		 n;
	Verb		 v;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		DS >> n;

		v.initialise();

		loadVerb( DS, v );

		v.setObject( O.id() );
		v.setName( n );

		Data.mVerbs.insert( n, v );
	}

	DS >> c;

	Property	 p;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		DS >> n;

		loadProperty( DS, p );

		p.setObject( O.id() );
		p.setName( n );

		Data.mProperties[ n ] = p;
	}

	Data.mConnection = -1;
}

void ODBFile::updateObject(QDataStream &DS, const Object &O)
{
	const ObjectData		&Data = data( O );

	DS << Data.mId;
	DS << Data.mPlayer;
	DS << Data.mParent;

	DS << quint32( Data.mChildren.size() );

	for( ObjectId O : Data.mChildren )
	{
		DS << O;
	}

	DS << Data.mName;
	DS << Data.mOwner;
	DS << Data.mLocation;

	DS << quint32( Data.mContents.size() );

	for( ObjectId O : Data.mContents )
	{
		DS << O;
	}

	DS << Data.mProgrammer;
	DS << Data.mWizard;
	DS << Data.mRead;
	DS << Data.mWrite;
	DS << Data.mFertile;

	DS << quint32( Data.mVerbs.size() );

	for( const QString &str : Data.mVerbs.keys() )
	{
		DS << str;

		saveVerb( DS, Data.mVerbs.value( str ) );
	}

	DS << quint32( Data.mProperties.size() );

	for( const QString &str : Data.mProperties.keys() )
	{
		DS << str;

		saveProperty( DS, Data.mProperties.value( str ) );
	}
}

void ODBFile::loadVerb(QDataStream &DS, Verb &V)
{
	FuncData		&FData = funcdata( V );
	VerbData		&VData = verbdata( V );

	quint16		D, I, P;

	DS >> FData.mOwner;
	DS >> FData.mRead;
	DS >> FData.mWrite;
	DS >> FData.mExecute;
	DS >> FData.mScript;

	FData.mDirty = true;

	DS >> D;
	DS >> I;
	DS >> P;
	DS >> VData.mPreposition;
	DS >> VData.mAliases;

	VData.mDirectObject    = ArgObj( D );
	VData.mIndirectObject  = ArgObj( I );
	VData.mPrepositionType = ArgObj( P );
}

void ODBFile::saveVerb(QDataStream &DS, const Verb &V)
{
	const FuncData		&FData = funcdata( V );
	const VerbData		&VData = verbdata( V );

	DS << FData.mOwner;
	DS << FData.mRead;
	DS << FData.mWrite;
	DS << FData.mExecute;
	DS << FData.mScript;

	DS << quint16( VData.mDirectObject );
	DS << quint16( VData.mIndirectObject );
	DS << quint16( VData.mPrepositionType );
	DS << VData.mPreposition;
	DS << VData.mAliases;
}

void ODBFile::loadProperty(QDataStream &DS, Property &P)
{
	PropertyData		&Data = data( P );

	DS >> Data.mOwner;
	DS >> Data.mRead;
	DS >> Data.mWrite;
	DS >> Data.mChange;
	DS >> Data.mValue;
	DS >> Data.mParent;
}

void ODBFile::saveProperty(QDataStream &DS, const Property &P)
{
	const PropertyData		&Data = data( P );

	DS << Data.mOwner;
	DS << Data.mRead;
	DS << Data.mWrite;
	DS << Data.mChange;
	DS << Data.mValue;
	DS << Data.mParent;
}

void ODBFile::loadTask( QDataStream &DS, TaskEntry &TE )
{
	TaskEntryData		&Data = data( TE );

	DS >> Data.mId;
	DS >> Data.mTimeStamp;
	DS >> Data.mCommand;
	DS >> Data.mPlayerId;
	DS >> Data.mConnectionId;
}

void ODBFile::saveTask( QDataStream &DS, const TaskEntry &TE )
{
	const TaskEntryData		&Data = data( TE );

	DS << Data.mId;
	DS << Data.mTimeStamp;
	DS << Data.mCommand;
	DS << Data.mPlayerId;
	DS << Data.mConnectionId;
}

void ODBFile::checkpoint()
{
	QString		DatStr = QDateTime::currentDateTime().toString( "yyyy-MM-dd.hh-mm-ss" );

	QFile( "moo.db" ).copy( QString( "%1.db" ).arg( DatStr ) );
}
