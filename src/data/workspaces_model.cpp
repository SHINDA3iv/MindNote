// #include "workspaces_model.h"

// WorkSpacesModel::WorkSpacesModel(QObject *parent) : QAbstractItemModel(parent)
// {}

// QModelIndex WorkSpacesModel::index(int row, int column, const QModelIndex &parent) const
// {
//     if (!parent.isValid() && row >= 0 && row < _workSpaces.size() && column >= 0 && column < 2) {
//         return createIndex(row, column);
//     }
//     return QModelIndex();
// }

// QModelIndex WorkSpacesModel::parent(const QModelIndex &index) const
// {
//     return QModelIndex();
// }

// int WorkSpacesModel::rowCount(const QModelIndex &parent) const
// {
//     return parent.isValid() ? 0 : _workSpaces.size();
// }

// int WorkSpacesModel::columnCount(const QModelIndex &parent) const
// {
//     return 2;
// }

// QVariant WorkSpacesModel::data(const QModelIndex &index, int role) const
// {
//     if (!index.isValid())
//         return QVariant();

//     const WorkSpace &workspace = _workSpaces.at(index.row());

//     if (role == Qt::DisplayRole) {
//         switch (index.column()) {
//         case 0:
//             return workspace.name();
//         case 1:
//             return workspace.link();
//         }
//     } else if (role == Qt::DecorationRole && index.column() == 0) {
//         return workspace.icon();
//     }

//     return QVariant();
// }

// Qt::ItemFlags WorkSpacesModel::flags(const QModelIndex &index) const
// {
//     if (!index.isValid())
//         return Qt::NoItemFlags;

//     return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
// }

// void WorkSpacesModel::addWorkSpace(const WorkSpace &workspace)
// {
//     beginInsertRows(QModelIndex(), _workSpaces.size(), _workSpaces.size());
//     _workSpaces.append(workspace);
//     endInsertRows();
// }
