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

    void saveWorkspace(Workspace *workspace, bool isGuest = false);
    Workspace *loadWorkspace(const QString &workspaceName, QWidget *parent = nullptr, bool isGuest = false);
    void deleteWorkspace(const QString &workspaceName, bool isGuest = false);
    void syncWorkspaces(const QJsonArray &serverWorkspaces, bool keepLocal = false);
    QString getWorkspacePath(bool isGuest = false) const;

private:
    QString storagePath;
    QString guestPath;
    QString userPath;
    void saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json);
    Workspace *loadWorkspaceRecursive(const QJsonObject &json, QWidget *parent = nullptr);
    void initializePaths();
};

#endif // LOCALSTORAGE_H
