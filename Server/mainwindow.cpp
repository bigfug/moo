#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QWindowStateChangeEvent>

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
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::log(const QString &pMessage)
{
	ui->plainTextEdit->appendPlainText( pMessage );
}

void MainWindow::installModel( QAbstractItemModel *pModel )
{
	Q_UNUSED( pModel )
	//ui->treeView->setModel( pModel );
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
