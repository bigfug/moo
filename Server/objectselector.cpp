#include "objectselector.h"
#include "ui_objectselector.h"

ObjectSelector::ObjectSelector(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ObjectSelector)
{
	ui->setupUi(this);
}

ObjectSelector::~ObjectSelector()
{
	delete ui;
}

void ObjectSelector::setObjectId(ObjectId pId)
{
	ui->mObjectSelectionId->setValue( pId );
}

void ObjectSelector::on_mObjectSelectionId_valueChanged(int arg1)
{
	emit objectSelected( arg1 );
}

void ObjectSelector::on_mObjectSelectionEdit_clicked()
{
	emit objectSelectedForEdit( ui->mObjectSelectionId->value() );
}
