#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QWindowStateChangeEvent>
#include "objectmanager.h"

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

	connect( ui->mObjectOwner, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectLocation, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mObjectParent, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );

	connect( ui->mVerbOwner, &ObjectSelector::objectSelectedForEdit, [=]( ObjectId pId )
	{
		setCurrentObject( pId );
	} );
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::log( const QString &pMessage )
{
	ui->plainTextEdit->appendPlainText( pMessage );
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

void MainWindow::on_mObjectId_valueChanged( int pId )
{
	setCurrentObject( pId );
}

void MainWindow::setCurrentObject( ObjectId pId )
{
	ui->mObjectId->setValue( pId );

	ui->mEditorStack->setCurrentIndex( 0 );

	ui->mVerbList->clear();
	ui->mPropList->clear();

	Object		*O = ObjectManager::o( pId );

	if( O )
	{
		for( QMap<QString,Verb>::const_iterator it = O->verbs().begin() ; it != O->verbs().end() ; it++ )
		{
			ui->mVerbList->addItem( it.value().name() );
		}

		for( QMap<QString,Property>::const_iterator it = O->properties().begin() ; it != O->properties().end() ; it++ )
		{
			ui->mPropList->addItem( it.value().name() );
		}

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

	Object		*O = ObjectManager::o( ui->mObjectId->value() );

	Verb		*V = O ? O->verb( item->text() ) : nullptr;

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

void MainWindow::on_mPropList_itemClicked(QListWidgetItem *item)
{
	ui->mEditorStack->setCurrentIndex( 2 );

	Object		*O = ObjectManager::o( ui->mObjectId->value() );

	Property	*P = O ? O->prop( item->text() ) : nullptr;

	if( P )
	{

	}
	else
	{

	}
}

void MainWindow::on_mObjectTree_clicked(const QModelIndex &index)
{
	setCurrentObject( index.internalId() );
}
