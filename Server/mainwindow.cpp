#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWindowStateChangeEvent>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "objectmanager.h"
#include "lua_object.h"
#include "lua_moo.h"

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
	
	ui->mPropertyParent->setEditorEnabled( false );

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

	connect( ui->mObjectModule, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectModule, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Object			*O = currentObject();

		if( O )
		{
			O->setModule( pId );
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

	connect( ui->mVerbOwner, &ObjectSelector::objectSelected, [=]( ObjectId pId )
	{
		Verb			*V = currentVerb();

		if( V )
		{
			V->setOwner( pId );
		}
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
	Object		*O = ObjectManager::o( ui->mCurrentObject->objectId() );

	return( O && O->valid() ? O : Q_NULLPTR );
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

bool MainWindow::isEditingObject() const
{
	return(	ui->mEditorStack->currentIndex() == 0 );
}

bool MainWindow::isEditingVerb() const
{
	return(	ui->mEditorStack->currentIndex() == 1 );
}

bool MainWindow::isEditingProperty() const
{
	return(	ui->mEditorStack->currentIndex() == 2 );
}

void MainWindow::installModel( QAbstractItemModel *pModel )
{
	ui->mObjectTree->setModel( pModel );
}

void MainWindow::stats( const ObjectManagerStats &pStats )
{
	ui->mStatusBar->showMessage( tr( "Tasks: %1 - Objects: %2 - Reads: %3 - Writes: %4 - MaxId: %5 - Execution: %6" )
								 .arg( pStats.mTasks )
								 .arg( pStats.mObjectCount )
								 .arg( pStats.mReads )
								 .arg( pStats.mWrites )
								 .arg( ObjectManager::instance()->maxId() )
								 .arg( pStats.mExecutionTime ) );
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
	QMessageBox::about( this, "About ArtMOO",
						QString( "ArtMOO v%1\n"
								 "\n"
								 "A text-only Multiuser Object Orientated server with technology extensions designed for art projects\n"
								 "\n"
								 "Written by Alex May\n"
								 "\n"
								 "https://github.com/bigfug/moo"
								 ).arg( QCoreApplication::applicationVersion() ) );
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
		ui->mObjectModule->setObjectId( O->module() );

		ui->mObjectRead->setChecked( O->read() );
		ui->mObjectPlayer->setChecked( O->player() );
		ui->mObjectWizard->setChecked( O->wizard() );
		ui->mObjectFertile->setChecked( O->fertile() );
		ui->mObjectProgrammer->setChecked( O->programmer() );

		ui->mButtonObjectExport->setEnabled(  O->module() != OBJECT_NONE );

		ui->mObjectEditor->setEnabled( true );
	}
	else
	{
		ui->mObjectEditor->setEnabled( false );

		ui->mObjectName->clear();
	}

	ui->mTabEditor->setEnabled( false );
}

void MainWindow::setCurrentVerb(QString pName)
{
	ui->mEditorStack->setCurrentIndex( 1 );

	QList<QListWidgetItem *>	L = ui->mVerbList->findItems( pName, Qt::MatchFixedString );

	if( !L.isEmpty() )
	{
		ui->mVerbList->setCurrentItem( L.first() );
	}

	updateVerb();
}

void MainWindow::setCurrentProperty(QString pName)
{
	ui->mEditorStack->setCurrentIndex( 2 );

	QList<QListWidgetItem *>	L = ui->mPropList->findItems( pName, Qt::MatchFixedString );

	if( !L.isEmpty() )
	{
		ui->mPropList->setCurrentItem( L.first() );
	}

	updateProperty();
}

void MainWindow::on_mVerbList_itemClicked(QListWidgetItem *)
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

	V.initialise();

	V.setOwner( ui->mCurrentOwner->objectId() );

	O->verbAdd( Name, V );

	setCurrentObject( O->id() );

	setCurrentVerb( Name );
}

void MainWindow::on_mButtonEditorUpdate_clicked()
{
	Object		*O = currentObject();

	if( !O )
	{
		return;
	}

	if( isEditingVerb() )
	{
		Verb		*V = currentVerb();

		Q_ASSERT( V );

		if( !V )
		{
			return;
		}

		if( verifyVerb() )
		{
			if( V->object() != O->id() )
			{
				Verb	V2 = *V;

				O->verbAdd( V->name(), V2 );

				V = currentVerb();

				Q_ASSERT( V );
			}

			V->setScript( ui->mTextEditor->document()->toPlainText() );

			setCurrentObject( O->id() );
			setCurrentVerb( V->name() );
		}
	}
	else if( isEditingProperty() )
	{
		Property	*P = currentProperty();

		if( !P )
		{
			return;
		}

		if( P->object() != O->id() )
		{
			Property	P2 = *P;

			O->propAdd( P->name(), P2 );

			P = currentProperty();

			Q_ASSERT( P );
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

								lua_util::stringsToObjects( VM );

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

		setCurrentObject( O->id() );
		setCurrentProperty( P->name() );
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

	setCurrentProperty( Name );
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

				lua_util::stringsToObjects( VM );
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
		ui->mVerbAliases->setText( V->aliases().join( ',' ) );

		ui->mVerbDirect->setCurrentIndex( V->directObject() );
		ui->mVerbIndirect->setCurrentIndex( V->indirectObject() );

		ui->mVerbRead->setChecked( V->read() );
		ui->mVerbWrite->setChecked( V->write() );
		ui->mVerbExecute->setChecked( V->execute() );

		QString		P = V->preposition();

		if( P.isEmpty() )
		{
			ui->mVerbPreposition->setCurrentIndex( V->prepositionType() - 1 );
		}
		else
		{
			ui->mVerbPreposition->setCurrentText( P );
		}

		ui->mTextEditor->setPlainText( V->script() );

		ui->mTextEditor->updateHighlighter();

		ui->mBottomTabs->setCurrentIndex( 1 );

		ui->mTabEditor->setEnabled( true );
	}
	else
	{
		ui->mVerbName->clear();
		ui->mVerbOwner->setObjectId( OBJECT_NONE );

		ui->mTextEditor->clear();

		ui->mTabEditor->setEnabled( false );
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
			ui->mTypeInvalid->setChecked( true );

			ui->mTextEditor->clear();
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

						lua_util::objectsToStrings( PrpDat );

						QJsonDocument	Json = QJsonDocument::fromVariant( PrpDat );

						ui->mTextEditor->setPlainText( Json.toJson( QJsonDocument::Indented ) );

						ui->mTypeMap->setChecked( true );
					}
					break;

				default:
					ui->mTypeInvalid->setChecked( true );

					qWarning() << "Unknown type" << P->value().typeName();

					ui->mTextEditor->clear();
					break;
			}
		}

		ui->mBottomTabs->setCurrentIndex( 1 );

		ui->mTabEditor->setEnabled( true );
	}
	else
	{
		ui->mPropEditor->setEnabled( false );

		ui->mTabEditor->setEnabled( false );
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

void MainWindow::on_mObjectName_editingFinished()
{
	Object		*O = currentObject();

	if( O )
	{
		QString		N = ui->mObjectName->text().trimmed();

		O->setName( N );
	}
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_mVerbPreposition_activated(int index)
{
	Verb		*V = currentVerb();

	if( V && index >= 0 && index <= 1 )
	{
		V->setPrepositionArgument( ArgObj( index + 1 ) );
	}
	else
	{
		V->setPrepositionArgument( ui->mVerbPreposition->currentText() );
	}
}

void MainWindow::on_mButtonEditorVerify_clicked()
{
	if( isEditingVerb() )
	{
		if( verifyVerb() )
		{
			QMessageBox::information( this, tr( "Verb code verify" ), tr( "No errors detected" ) );
		}
	}
}

bool MainWindow::verifyVerb()
{
	Verb			*V = currentVerb();

	if( !V )
	{
		return( false );
	}

	lua_State		*L = lua_moo::luaNewState();

	QByteArray		 P = ui->mTextEditor->document()->toPlainText().toUtf8();

	int Error = luaL_loadbuffer( L, P, P.size(), V->name().toUtf8() );

	if( Error )
	{
		QString		Result = lua_tostring( L, -1 );

		QMessageBox::warning( this, tr( "Verb code verify" ), Result );

		lua_pop( L, 1 );
	}

	lua_close( L );

	return( !Error );
}

void MainWindow::on_mButtonObjectExport_clicked()
{
	QSettings		 S;
	Object			*O = currentObject();

	if( !O )
	{
		return;
	}

	QString			 N = QFileDialog::getSaveFileName( this, "Export Module", S.value( "export_module_directory", QDir::currentPath() ).toString(), "*.db" );

	if( !N.isEmpty() )
	{
		TransferInformation		TrnInf;

		ObjectManager::instance()->exportModule( O->module(), N, TrnInf );

		QMessageBox::information( this, "Export Information",
								  QString( "Exported %1 objects, %2 verbs, and %3 properties in %4 milliseconds" )
								  .arg( TrnInf.mObjects ).arg( TrnInf.mVerbs ).arg( TrnInf.mProperties )
								  .arg( TrnInf.mMilliseconds ) );

		S.setValue( "export_module_directory", QFileInfo( N ).path() );
	}
}

void MainWindow::on_mButtonObjectImport_clicked()
{
	QSettings		 S;
	Object			*O = currentObject();

	if( !O )
	{
		return;
	}

	QString			 N = QFileDialog::getOpenFileName( this, "Import Module", S.value( "import_module_directory", QDir::currentPath() ).toString(), "*.db" );

	if( !N.isEmpty() )
	{
		TransferInformation		TrnInf;

		ObjectManager::instance()->importModule( O->id(), ui->mCurrentOwner->objectId(), N, TrnInf );

		QMessageBox::information( this, "Import Information",
								  QString( "Imported %1 objects, %2 verbs, and %3 properties in %4 milliseconds" )
								  .arg( TrnInf.mObjects ).arg( TrnInf.mVerbs ).arg( TrnInf.mProperties )
								  .arg( TrnInf.mMilliseconds ) );

		S.setValue( "import_module_directory", QFileInfo( N ).path() );
	}
}

void MainWindow::on_mVerbRead_clicked(bool checked)
{
	Verb		*V = currentVerb();

	if( V )
	{
		V->setRead( checked );
	}
}

void MainWindow::on_mVerbWrite_clicked(bool checked)
{
	Verb		*V = currentVerb();

	if( V )
	{
		V->setWrite( checked );
	}
}

void MainWindow::on_mVerbExecute_clicked(bool checked)
{
	Verb		*V = currentVerb();

	if( V )
	{
		V->setExecute( checked );
	}
}
