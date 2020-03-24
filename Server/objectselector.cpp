#include "objectselector.h"
#include "ui_objectselector.h"
#include "objectmanager.h"

ObjectSelector::ObjectSelector(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ObjectSelector)
{
	ui->setupUi(this);

	ui->mObjectSelectionId->setMinimum( -1 );
	ui->mObjectSelectionId->setMaximum( 9999999 );

	ui->mObjectSelectionId->setValue( -1 );
}

ObjectSelector::~ObjectSelector()
{
	delete ui;
}

ObjectId ObjectSelector::objectId() const
{
	return( ObjectId( ui->mObjectSelectionId->value() ) );
}

void ObjectSelector::setObjectId(ObjectId pId)
{
	ui->mObjectSelectionId->setValue( pId );
}

void ObjectSelector::on_mObjectSelectionId_valueChanged(int arg1)
{
	if( arg1 == -1 )
	{
		ui->mObjectSelectionId->setSuffix( " - NONE" );
	}
	else
	{
		QString		ObjNam = ObjectManager::instance()->objectName( arg1 );

		if( ObjNam.isEmpty() )
		{
			ui->mObjectSelectionId->setSuffix( " - INVALID" );
		}
		else
		{
			ui->mObjectSelectionId->setSuffix( QString( " - %1" ).arg( ObjNam ) );
		}
	}

	emit objectSelected( arg1 );
}

void ObjectSelector::on_mObjectSelectionEdit_clicked()
{
	emit objectSelectedForEdit( ui->mObjectSelectionId->value() );
}