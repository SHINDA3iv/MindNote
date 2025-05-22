#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QObject>
#include <QDir>
#include "workspace.h"

class LocalStorage : public QObject
{
    Q_OBJECT
public:
    explicit LocalStorage(QObject *parent = nullptr);

    void saveWorkspace(Workspace *workspace);
    Workspace *loadWorkspace(const QString &workspaceName, QWidget *parent = nullptr);
    void deleteWorkspace(const QString &workspaceName);

private:
    QString storagePath;
    void saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json);
    Workspace *loadWorkspaceRecursive(const QJsonObject &json, QWidget *parent = nullptr);
};
#endif // LOCALSTORAGE_H
