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
    Workspace *
    loadWorkspace(const QString &workspaceTitle, QWidget *parent = nullptr, bool isGuest = false);
    void deleteWorkspace(const QString &workspaceTitle, bool isGuest = false);
    void syncWorkspaces(const QJsonArray &serverWorkspaces, bool keepLocal = false);
    QString getWorkspacePath(bool isGuest = false) const;
    void setCurrentUser(const QString &username);
    QString getCurrentUser() const;
    void clearUserData();
    QString getWorkspaceOwnerPath(const QString &ownerUsername) const;

private:
    QString storagePath;
    QString guestPath;
    QString userPath;
    QString currentUser;
    void saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json);
    Workspace *loadWorkspaceRecursive(const QJsonObject &json, QWidget *parent = nullptr);
    void initializePaths();
    QString getUserWorkspacePath() const;
};

#endif // LOCALSTORAGE_H
