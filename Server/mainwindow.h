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

	Verb *currentVerb( void );

	Property *currentProperty( void );

	static void objectsToStrings( QVariantMap &PrpDat );

	static void stringsToObjects( QVariantMap &PrpDat );

	bool isEditingObject( void ) const;
	bool isEditingVerb( void ) const;
	bool isEditingProperty( void ) const;

private slots:
	void trayActivated( QSystemTrayIcon::ActivationReason pReason );

	void on_actionAbout_triggered();

	void setCurrentObject( ObjectId pId );

	void setCurrentVerb( QString pName );

	void setCurrentProperty( QString pName );

	void on_mVerbList_itemClicked(QListWidgetItem *item);

	void on_mPropList_itemClicked(QListWidgetItem *item);

	void on_mObjectTree_clicked(const QModelIndex &index);

	void on_mButtonObjectAdd_clicked();

	void on_mButtonVerbAdd_clicked();

	void on_mButtonEditorUpdate_clicked();

	void on_mButtonPropertyAdd_clicked();

	void on_mTypeNumber_clicked();

	void on_mTypeString_clicked();

	void on_mTypeBoolean_clicked();

	void updateVerbList( void );

	void updatePropList( void );

	void updateVerb( void );

	void updateProperty( void );

	void on_mTypeObject_clicked();

	void on_mTypeMap_clicked();

	void on_mPropertyName_textEdited(const QString &arg1);

	void on_mPropertyRead_clicked(bool checked);

	void on_mPropertyWrite_clicked(bool checked);

	void on_mPropertyChange_clicked(bool checked);

	void on_mButtonPropertyClear_clicked();

	void on_mButtonPropertyDelete_clicked();

	void on_mButtonVerbDelete_clicked();

	void on_mObjectAliases_textEdited(const QString &arg1);

	void on_mVerbAliases_textEdited(const QString &arg1);

	void on_mObjectPlayer_clicked(bool checked);

	void on_mObjectProgrammer_clicked(bool checked);

	void on_mObjectWizard_clicked(bool checked);

	void on_mObjectRead_clicked(bool checked);

	void on_mObjectWrite_clicked(bool checked);

	void on_mObjectFertile_clicked(bool checked);

	void on_mVerbDirect_currentIndexChanged(int index);

	void on_mVerbIndirect_currentIndexChanged(int index);

	void on_mButtonObjectDelete_clicked();

	void on_mObjectName_editingFinished();

	void on_actionExit_triggered();

	void on_mVerbPreposition_activated(int index);

	void on_mButtonEditorVerify_clicked();

	bool verifyVerb( void );

	void on_mButtonObjectExport_clicked();

	void on_mButtonObjectImport_clicked();

private:
	bool event( QEvent *pEvent );

private:
	Ui::MainWindow		*ui;
	QSystemTrayIcon		*mTrayIcon;
};

#endif // MAINWINDOW_H
