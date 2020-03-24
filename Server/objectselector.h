#ifndef OBJECTSELECTOR_H
#define OBJECTSELECTOR_H

#include <QWidget>

#include "mooglobal.h"

namespace Ui {
class ObjectSelector;
}

class ObjectSelector : public QWidget
{
	Q_OBJECT

public:
	explicit ObjectSelector(QWidget *parent = nullptr);

	virtual ~ObjectSelector();

	ObjectId objectId( void ) const;

public slots:
	void setObjectId( ObjectId pId );

signals:
	void objectSelectedForEdit( ObjectId pId );

	void objectSelected( ObjectId pId );

private slots:
	void on_mObjectSelectionId_valueChanged(int arg1);

	void on_mObjectSelectionEdit_clicked();

private:
	Ui::ObjectSelector *ui;
};

#endif // OBJECTSELECTOR_H
