#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QWindowStateChangeEvent>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>

#include "objectmanager.h"
#include "lua_object.h"

MainWindow::MainWindow( QWidget *pParent )
	: QMainWindow( pParent ), ui( new Ui::MainWindow )
{
	ui->setupUi( this );

	setWindowIcon( QIcon( ":/moo-icon.ico" ) );

	setWindowTitle( QString( "%1 v%2 by %3 - %4" ).arg( qApp->applicationName() ).arg( qApp->applicationVersion() ).arg( qApp->organizationName() ).arg( qApp->organizationDomain() ) );

	mTrayIcon = new QSystemTrayIcon( QIcon( ":/moo-icon.ico" ), this );

	connect( mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

	mTrayIcon->setToolTip( "ArtMOO" );

	mTrayIcon->show();

	connect( ui->mCurrentObject, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mCurrentOwner, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		QSettings().setValue( "owner", int( pId ) );
	} );

	connect( ui->mCurrentOwner, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectOwner, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectOwner, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Object			*O = currentObject();

		if( O )
		{
			O->setOwner( pId );
		}
	} );

	connect( ui->mObjectLocation, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectLocation, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Object			*O = currentObject();
		Object			*D = ObjectManager::o( pId );

		if( O )
		{
			O->move( D );
		}
	} );

	connect( ui->mObjectParent, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectParent, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Object			*O = currentObject();

		if( O )
		{
			O->setParent( pId );
		}
	} );

	connect( ui->mVerbOwner, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mPropertyOwner, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Property		*P = currentProperty();

		if( P )
		{
			P->setOwner( pId );
		}
	} );

	ui->mCurrentOwner->setObjectId( QSettings().value( "owner", ui->mCurrentOwner->objectId() ).toInt() );

	connect( ObjectManager::instance(), &ObjectManager::stats, this, &MainWindow::stats );
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::log( const QString &pMessage )
{
	ui->plainTextEdit->appendPlainText( pMessage );
}

Object *MainWindow::currentObject()
{
	return( ObjectManager::o( ui->mCurrentObject->objectId() )  );
}

Verb *MainWindow::currentVerb()
{
	QListWidgetItem		*VerbItem = ui->mVerbList->currentItem();

	if( !VerbItem )
	{
		return( nullptr );
	}

	Object		*O = currentObject();
	Object		*FO = nullptr;
	Verb		*FV = nullptr;

	if( !O || !O->verbFind( VerbItem->text(), &FV, &FO ) )
	{
		return( nullptr );
	}

	return( FV );
}

Property *MainWindow::currentProperty()
{
	QListWidgetItem		*PropItem = ui->mPropList->currentItem();

	if( !PropItem )
	{
		return( nullptr );
	}

	Object		*O = currentObject();
	Object		*FO = nullptr;
	Property	*FP = nullptr;

	if( !O || !O->propFind( PropItem->text(), &FP, &FO ) )
	{
		return( nullptr );
	}

	return( FP );
}

void MainWindow::objectsToStrings( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( it.value().canConvert<lua_object::luaHandle>() )
		{
			lua_object::luaHandle	LH = it.value().value<lua_object::luaHandle>();

			it.value() = QString( "#%1" ).arg( LH.O );
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QString )
		{
			QString					ST = it.value().toString();

			if( ST.startsWith( '#' ) )
			{
				ST.prepend( '#' );

				it.value() = ST;
			}
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QVariantMap )
		{
			QVariantMap		VM = it.value().toMap();

			objectsToStrings( VM );

			it.value() = VM;
		}
	}
}

void MainWindow::stringsToObjects( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( QMetaType::Type( it.value().type() ) == QMetaType::QString )
		{
			QString					ST = it.value().toString();

			if( ST.startsWith( "##" ) )
			{
				ST.remove( 0, 1 );

				it.value() = ST;
			}
			else if( ST.startsWith( '#' ) )
			{
				ST.remove( 0, 1 );

				lua_object::luaHandle	LH;

				LH.O = ST.toInt();

				it.value() = QVariant::fromValue( LH );
			}
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QVariantMap )
		{
			QVariantMap		VM = it.value().toMap();

			stringsToObjects( VM );

			it.value() = VM;
		}
	}
}

void MainWindow::installModel( QAbstractItemModel *pModel )
{
	ui->mObjectTree->setModel( pModel );
}

void MainWindow::stats( const ObjectManagerStats &pStats )
{
	ui->mStatusBar->showMessage( tr( "Tasks: %1 - Objects: %2 - Reads: %3 - Writes: %4" ).arg( pStats.mTasks ).arg( pStats.mObjectCount ).arg( pStats.mReads ).arg( pStats.mWrites ) );
}

void MainWindow::trayActivated( QSystemTrayIcon::ActivationReason pReason )
{
	if( pReason == QSystemTrayIcon::Trigger )
	{
		if( isMinimized() )
		{
			setParent( NULL, Qt::Window );

			showNormal();
		}
	}
}

bool MainWindow::event( QEvent *pEvent )
{
	if( pEvent->type() == QEvent::WindowStateChange )
	{
		if( isMinimized() )
		{
			setParent( NULL, Qt::SubWindow );

			pEvent->ignore();
		}
		else
		{
			pEvent->accept();
		}
	}

	return( QMainWindow::event( pEvent ) );
}

void MainWindow::on_actionAbout_triggered()
{

}

void MainWindow::setCurrentObject( ObjectId pId )
{
	ui->mCurrentObject->setObjectId( pId );

	ui->mEditorStack->setCurrentIndex( 0 );

	updateVerbList();
	updatePropList();

	Object		*O = ObjectManager::o( pId );

	if( O )
	{
		ui->mObjectName->setText( O->name() );
		ui->mObjectOwner->setObjectId( O->owner() );
		ui->mObjectParent->setObjectId( O->parent() );
		ui->mObjectLocation->setObjectId( O->location() );
		ui->mObjectAliases->setText( O->aliases().join( ',' ) );

		ui->mObjectRead->setChecked( O->read() );
		ui->mObjectPlayer->setChecked( O->player() );
		ui->mObjectWizard->setChecked( O->wizard() );
		ui->mObjectFertile->setChecked( O->fertile() );
		ui->mObjectProgrammer->setChecked( O->programmer() );

		ui->mObjectEditor->setEnabled( true );
	}
	else
	{
		ui->mObjectEditor->setEnabled( false );

		ui->mObjectName->clear();
	}
}

void MainWindow::on_mVerbList_itemClicked(QListWidgetItem *item)
{
	ui->mEditorStack->setCurrentIndex( 1 );

	updateVerb();
}

void MainWindow::on_mPropList_itemClicked(QListWidgetItem *)
{
	ui->mEditorStack->setCurrentIndex( 2 );

	updateProperty();
}

void MainWindow::on_mObjectTree_clicked(const QModelIndex &index)
{
	setCurrentObject( index.internalId() );
}

void MainWindow::on_mButtonObjectAdd_clicked()
{
	bool		OK;
    QString		Name = QInputDialog::getText( this, tr( "Object name" ), tr( "Object name" ), QLineEdit::Normal, QString(), &OK );

	if( OK && !Name.isEmpty() )
	{
		Object		*O = currentObject();

		Object		*N = ObjectManager::instance()->newObject();

		N->setName( Name );
		N->setParent( O->id() );
		N->setOwner( ui->mCurrentOwner->objectId() );

		setCurrentObject( N->id() );
	}
}


void MainWindow::on_mButtonObjectDelete_clicked()
{
	Object		*O = currentObject();

	if( !O )
	{
		return;
	}

	ObjectManager::instance()->recycle( O );

	ui->mObjectTree->update();
}

void MainWindow::on_mButtonVerbAdd_clicked()
{
	Object		*O = currentObject();

	if( !O )
	{
		return;
	}

	bool		OK;
    QString		Name = QInputDialog::getText( this, tr( "Verb name" ), tr( "Verb name" ), QLineEdit::Normal, QString(), &OK );

	if( !OK || Name.isEmpty() )
	{
		return;
	}

	Verb		*FndVrb;
	Object		*FndObj;

	if( O->verbFind( Name, &FndVrb, &FndObj ) )
	{
		return;
	}

	Verb		 V;

	V.setOwner( ui->mCurrentOwner->objectId() );

	O->verbAdd( Name, V );

	setCurrentObject( O->id() );
}

void MainWindow::on_mButtonEditorUpdate_clicked()
{
	Object		*O = currentObject();

	if( !O )
	{
		return;
	}

	if( ui->mEditorStack->currentIndex() == 1 )
	{
		Verb		*V = currentVerb();

		if( !V )
		{
			return;
		}

		Verb		 N = *V;

		N.setScript( ui->mTextEditor->document()->toPlainText() );

		O->verbAdd( V->name(), N );
	}
	else if( ui->mEditorStack->currentIndex() == 2 )
	{
		Property	*P = currentProperty();

		if( !P )
		{
			return;
		}

		QString		 S = ui->mTextEditor->document()->toPlainText();

		if( P->type() == QVariant::Invalid )
		{
			ui->mTextEditor->setPlainText( QString() );
		}
		else if( !strcmp( P->value().typeName(), "lua_object::luaHandle" ) )
		{
			lua_object::luaHandle	LH;

			LH.O = QVariant( S ).value<int>();

			O->propSet( P->name(), QVariant::fromValue( LH ) );
		}
		else
		{
			switch( QMetaType::Type( P->type() ) )
			{
				case QMetaType::Bool:
					O->propSet( P->name(), QVariant( S ).value<bool>() );
					break;

				case QMetaType::Int:
					O->propSet( P->name(), QVariant( S ).value<int>() );
					break;

				case QMetaType::Double:
					O->propSet( P->name(), QVariant( S ).value<double>() );
					break;

				case QMetaType::QString:
					O->propSet( P->name(), S );
					break;

				case QMetaType::QVariantMap:
					{
						QJsonDocument		J;
						QJsonParseError		E;

						J = QJsonDocument::fromJson( S.toLatin1(), &E );

						if( E.error == QJsonParseError::NoError )
						{
							QVariant		V = J.toVariant();

							if( QMetaType::Type( V.type() ) == QMetaType::QVariantMap )
							{
								QVariantMap		VM = V.toMap();

								stringsToObjects( VM );

								O->propSet( P->name(), VM );
							}
						}
						else
						{
							qWarning() << E.errorString();
						}
					}
					break;

				default:
					ui->mTextEditor->setPlainText( QString() );
					break;
			}
		}
	}
}

void MainWindow::on_mButtonPropertyAdd_clicked()
{
	Object		*O = currentObject();

	if( !O )
	{
		return;
	}

	bool		OK;
    QString		Name = QInputDialog::getText( this, tr( "Property name" ), tr( "Property name" ), QLineEdit::Normal, QString(), &OK );

	if( !OK || Name.isEmpty() )
	{
		return;
	}

	Property	*FndPrp;
	Object		*FndObj;

	if( O->propFind( Name, &FndPrp, &FndObj ) )
	{
		return;
	}

	Property		 P;

	P.setOwner( ui->mCurrentOwner->objectId() );

	O->propAdd( Name, P );

	setCurrentObject( O->id() );
}

void MainWindow::on_mTypeNumber_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	QVariant	 V;

	if( P->value().canConvert<double>() )
	{
		V = P->value().value<double>();
	}
	else
	{
		V = QVariant::fromValue<double>( 0 );
	}

	O->propSet( P->name(), V );

	updateProperty();
}

void MainWindow::on_mTypeString_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	QVariant	 V;

	if( P->value().canConvert<QString>() )
	{
		V = P->value().value<QString>();
	}
	else
	{
		V = QString();
	}

	O->propSet( P->name(), V );

	updateProperty();
}

void MainWindow::on_mTypeBoolean_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	QVariant	 V;

	if( P->value().canConvert<bool>() )
	{
		V = P->value().value<bool>();
	}
	else
	{
		V = QVariant::fromValue<bool>( false );
	}

	O->propSet( P->name(), V );

	updateProperty();
}

void MainWindow::on_mTypeObject_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	lua_object::luaHandle	LH;

	if( P->value().canConvert<int>() )
	{
		LH.O = P->value().value<int>();
	}
	else
	{
		LH.O = OBJECT_NONE;
	}

	O->propSet( P->name(), QVariant::fromValue( LH ) );

	updateProperty();
}

void MainWindow::on_mTypeMap_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	QVariantMap		VM;

	if( P->value().canConvert<QString>() )
	{
		QJsonDocument		J;
		QJsonParseError		E;

		J = QJsonDocument::fromJson( P->value().value<QString>().toLatin1(), &E );

		if( E.error == QJsonParseError::NoError )
		{
			QVariant		V = J.toVariant();

			if( QMetaType::Type( V.type() ) == QMetaType::QVariantMap )
			{
				VM = V.toMap();

				stringsToObjects( VM );
			}
		}
		else
		{
			qWarning() << E.errorString();
		}
	}

	O->propSet( P->name(), VM );

	updateProperty();
}

void MainWindow::updateVerbList()
{
	ui->mVerbList->clear();

	Object	*O = currentObject();

	if( O )
	{
		QStringList		VrbLst = O->verbs().keys();

		for( ObjectId OID : O->ancestors() )
		{
			Object	*P = ObjectManager::o( OID );

			for( QMap<QString,Verb>::const_iterator it = P->verbs().begin() ; it != P->verbs().end() ; it++ )
			{
				QListWidgetItem		*ListItem = new QListWidgetItem( it.value().name() );

				if( !VrbLst.contains( it.value().name() ) )
				{
					ListItem->setForeground( QColor( Qt::lightGray ) );
				}
				else
				{
					VrbLst.removeAll( it.value().name() );
				}

				ui->mVerbList->addItem( ListItem );
			}
		}

		for( QString PN : VrbLst )
		{
			ui->mVerbList->addItem( PN );
		}
	}
}

void MainWindow::updatePropList()
{
	ui->mPropList->clear();

	Object	*O = currentObject();

	if( O )
	{
		QStringList		PrpLst = O->properties().keys();

		for( ObjectId OID : O->ancestors() )
		{
			Object	*P = ObjectManager::o( OID );

			for( QMap<QString,Property>::const_iterator it = P->properties().begin() ; it != P->properties().end() ; it++ )
			{
				QListWidgetItem		*ListItem = new QListWidgetItem( it.value().name() );

				if( !PrpLst.contains( it.value().name() ) )
				{
					ListItem->setForeground( QColor( Qt::lightGray ) );
				}
				else
				{
					PrpLst.removeAll( it.value().name() );
				}

				ui->mPropList->addItem( ListItem );
			}
		}

		for( QString PN : PrpLst )
		{
			ui->mPropList->addItem( PN );
		}
	}
}

void MainWindow::updateVerb()
{
	Verb		*V = currentVerb();

	if( V )
	{
		ui->mVerbName->setText( V->name() );
		ui->mVerbOwner->setObjectId( V->owner() );

		ui->mTextEditor->setPlainText( V->script() );

		ui->mBottomTabs->setCurrentIndex( 1 );
	}
	else
	{
		ui->mVerbName->clear();
		ui->mVerbOwner->setObjectId( OBJECT_NONE );

		ui->mTextEditor->clear();
	}
}

void MainWindow::updateProperty( void )
{
	Property	*P = currentProperty();

	if( P )
	{
		ui->mPropEditor->setEnabled( true );

		ui->mPropertyName->setText( P->name() );
		ui->mPropertyOwner->setObjectId( P->owner() );
		ui->mPropertyParent->setObjectId( P->parent() );
		ui->mPropertyRead->setChecked( P->read() );
		ui->mPropertyWrite->setChecked( P->write() );
		ui->mPropertyChange->setChecked( P->change() );

		if( P->type() == QVariant::Invalid )
		{
			ui->mTypeBoolean->setChecked( false );
			ui->mTypeNumber->setChecked( false );
			ui->mTypeString->setChecked( false );
			ui->mTypeMap->setChecked( false );
			ui->mTypeObject->setChecked( false );
		}
		else if( !strcmp( P->value().typeName(), "lua_object::luaHandle" ) )
		{
			lua_object::luaHandle	LH = P->value().value<lua_object::luaHandle>();

			ui->mTypeObject->setChecked( true );

			ui->mTextEditor->setPlainText( QString::number( LH.O ) );
		}
		else
		{
			switch( QMetaType::Type( P->type() ) )
			{
				case QMetaType::Bool:
					ui->mTypeBoolean->setChecked( true );
					ui->mTextEditor->setPlainText( P->value().toBool() ? "true" : "false" );
					break;

				case QMetaType::Int:
					ui->mTypeNumber->setChecked( true );
					ui->mTextEditor->setPlainText( QString::number( P->value().toInt() ) );
					break;

				case QMetaType::Double:
					ui->mTypeNumber->setChecked( true );
					ui->mTextEditor->setPlainText( QString::number( P->value().toDouble() ) );
					break;

				case QMetaType::QString:
					ui->mTypeString->setChecked( true );
					ui->mTextEditor->setPlainText( P->value().toString() );
					break;

				case QMetaType::QVariantMap:
					{
						QVariantMap		PrpDat = P->value().toMap();

						objectsToStrings( PrpDat );

						QJsonDocument	Json = QJsonDocument::fromVariant( PrpDat );

						ui->mTextEditor->setPlainText( Json.toJson( QJsonDocument::Indented ) );

						ui->mTypeMap->setChecked( true );
					}
					break;

				default:
					qWarning() << "Unknown type" << P->value().typeName();
					ui->mTextEditor->setPlainText( QString() );
					break;
			}
		}
	}
	else
	{
		ui->mPropEditor->setEnabled( false );
	}
}

void MainWindow::on_mPropertyName_textEdited( const QString &pName )
{
	if( pName.isEmpty() )
	{
		qWarning() << "Property name cannot be empty";

		return;
	}

	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	if( P->name() == pName )
	{
		return;
	}

	Object		*FO;
	Property	*FP;

	if( O->propFind( pName, &FP, &FO ) )
	{
		qWarning() << "An existing property called" << pName << "was found on object #" << FO->id() << FO->name();

		return;
	}

	P->setName( pName );

	updatePropList();
}

void MainWindow::on_mPropertyRead_clicked(bool checked)
{
	Property	*P = currentProperty();

	if( !P )
	{
		return;
	}

	P->setRead( checked );
}

void MainWindow::on_mPropertyWrite_clicked(bool checked)
{
	Property	*P = currentProperty();

	if( !P )
	{
		return;
	}

	P->setWrite( checked );
}

void MainWindow::on_mPropertyChange_clicked(bool checked)
{
	Property	*P = currentProperty();

	if( !P )
	{
		return;
	}

	P->setChange( checked );
}

void MainWindow::on_mButtonPropertyClear_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	O->propClear( P->name() );

	updatePropList();

	updateProperty();
}

void MainWindow::on_mButtonPropertyDelete_clicked()
{
	Object		*O = currentObject();
	Property	*P = currentProperty();

	if( !O || !P )
	{
		return;
	}

	O->propDelete( P->name() );

	updatePropList();

	updateProperty();
}

void MainWindow::on_mButtonVerbDelete_clicked()
{
	Object		*O = currentObject();
	Verb		*V = currentVerb();

	if( !O || !V )
	{
		return;
	}

	O->verbDelete( V->name() );

	updateVerbList();

	updateVerb();
}

void MainWindow::on_mObjectAliases_textEdited(const QString &arg1)
{
	Object		*O = currentObject();

	if( O )
	{
		QStringList		P = O->aliases();
		QStringList		L = arg1.split( ',' );

		for( QString S : L )
		{
			S = S.trimmed();

			if( !P.contains( S ) )
			{
				O->aliasAdd( S );
			}
			else
			{
				P.removeAll( S );
			}
		}

		for( QString S : P )
		{
			O->aliasDelete( S );
		}
	}
}

void MainWindow::on_mVerbAliases_textEdited(const QString &arg1)
{
	Verb		*V = currentVerb();

	if( V )
	{
		QStringList		P = V->aliases();
		QStringList		L = arg1.split( ',' );

		for( QString S : L )
		{
			S = S.trimmed();

			if( !P.contains( S ) )
			{
				V->addAlias( S );
			}
			else
			{
				P.removeAll( S );
			}
		}

		for( QString S : P )
		{
			V->remAlias( S );
		}
	}
}

void MainWindow::on_mObjectPlayer_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setPlayer( checked );
	}
}

void MainWindow::on_mObjectProgrammer_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setProgrammer( checked );
	}
}

void MainWindow::on_mObjectWizard_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setWizard( checked );
	}
}

void MainWindow::on_mObjectRead_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setRead( checked );
	}
}

void MainWindow::on_mObjectWrite_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setWrite( checked );
	}
}

void MainWindow::on_mObjectFertile_clicked(bool checked)
{
	Object		*O = currentObject();

	if( O )
	{
		O->setFertile( checked );
	}
}

void MainWindow::on_mVerbDirect_currentIndexChanged(int index)
{
	Verb		*V = currentVerb();

	if( V )
	{
		V->setDirectObjectArgument( ArgObj( index ) );
	}
}

void MainWindow::on_mVerbIndirect_currentIndexChanged(int index)
{
	Verb		*V = currentVerb();

	if( V )
	{
		V->setIndirectObjectArgument( ArgObj( index ) );
	}
}

void MainWindow::on_mVerbPreposition_currentIndexChanged(int index)
{
	Verb		*V = currentVerb();

	if( V && index <= 2 )
	{
		V->setPrepositionArgument( ArgObj( index ) );
	}
}

void MainWindow::on_mObjectName_editingFinished()
{
	Object		*O = currentObject();

	if( O )
	{
		QString		N = ui->mObjectName->text().trimmed();

		O->setName( N );
	}
}
