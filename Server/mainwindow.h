#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAbstractItemModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow( QWidget *parent = 0 );
	virtual ~MainWindow( void );

	void log( const QString &pMessage );

	void installModel( QAbstractItemModel *pModel );

public slots:
	void stats( int pTaskCount, int pObjectCount );

private slots:
	void trayActivated( QSystemTrayIcon::ActivationReason pReason );

	void on_actionAbout_triggered();

private:
	bool event( QEvent *pEvent );

private:
	Ui::MainWindow		*ui;
	QSystemTrayIcon		*mTrayIcon;
};

#endif // MAINWINDOW_H
