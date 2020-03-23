#ifndef MOOITEMMODEL_H
#define MOOITEMMODEL_H

#include <QAbstractItemModel>

#include "mooglobal.h"

class MooItem
{
public:
	MooItem( ObjectId pId )
		: mId( pId )
	{

	}

private:
	ObjectId		mId;
};

class MooItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	MooItemModel( QObject *pParent = Q_NULLPTR );

	// QAbstractItemModel interface
public:
	virtual QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

	// QAbstractItemModel interface
public:
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
};

#endif // MOOITEMMODEL_H
