#ifndef WORKSPACES_MODEL_H
#define WORKSPACES_MODEL_H

// #include "workspace.h"

// #include <QAbstractItemModel>
// #include <QVector>

// class WorkSpacesModel : public QAbstractItemModel
// {
//     Q_OBJECT
// public:
//     explicit WorkSpacesModel(QObject *parent = nullptr);
//     ~WorkSpacesModel() override = default;

//     QModelIndex
//     index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
//     QModelIndex parent(const QModelIndex &index) const override;
//     int rowCount(const QModelIndex &parent = QModelIndex()) const override;
//     int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//     QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
//     Qt::ItemFlags flags(const QModelIndex &index) const override;

//     void addWorkSpace(const Workspace &workspace);

// private:
//     QVector<Workspace> _workSpaces;
// };

#endif // WORKSPACES_MODEL_H
