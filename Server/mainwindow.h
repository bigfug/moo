#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAbstractItemModel>
#include <QListWidgetItem>

#include "objectmanager.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow( QWidget *parent = 0 );

	virtual ~MainWindow( void );

	void installModel( QAbstractItemModel *pModel );

public slots:
	void stats( const ObjectManagerStats &pStats );

	void log( const QString &pMessage );

private:
	Object *currentObject( void );

private slots:
	void trayActivated( QSystemTrayIcon::ActivationReason pReason );

	void on_actionAbout_triggered();

	void on_mObjectId_valueChanged(int arg1);

	void setCurrentObject( ObjectId pId );

	void on_mVerbList_itemClicked(QListWidgetItem *item);

	void on_mPropList_itemClicked(QListWidgetItem *item);

	void on_mObjectTree_clicked(const QModelIndex &index);

	void on_mButtonObjectAdd_clicked();

	void on_mButtonVerbAdd_clicked();

	void on_mButtonEditorUpdate_clicked();

	void on_mButtonPropertyAdd_clicked();

private:
	bool event( QEvent *pEvent );

private:
	Ui::MainWindow		*ui;
	QSystemTrayIcon		*mTrayIcon;
};

#endif // MAINWINDOW_H
