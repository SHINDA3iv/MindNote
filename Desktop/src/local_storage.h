#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <QObject>
#include <QSettings>
#include <QJsonObject>

class LocalStorage : public QObject
{
    Q_OBJECT
public:
    explicit LocalStorage(QObject *parent = nullptr);

    void saveWorkspaces(const QJsonArray &workspaces);
    QJsonArray loadWorkspaces() const;

    void saveWorkspaceItems(const QString &workspaceId, const QJsonArray &items);
    QJsonArray loadWorkspaceItems(const QString &workspaceId) const;

    QDateTime lastSyncTime() const;
    void setLastSyncTime(const QDateTime &time);

private:
    QString storagePath;
};

#endif // LOCALSTORAGE_H
