#include "local_storage.h"
#include <qdir.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

LocalStorage::LocalStorage(QObject *parent) : QObject(parent)
{
    storagePath =
     QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/workspaces/";
}

void LocalStorage::saveWorkspaces(const QJsonArray &workspaces)
{
    QFile file(storagePath + "workspaces.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(workspaces).toJson());
    }
}

QJsonArray LocalStorage::loadWorkspaces() const
{
    QFile file(storagePath + "workspaces.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    return QJsonDocument::fromJson(data).array();
}

void LocalStorage::saveWorkspaceItems(const QString &workspaceId, const QJsonArray &items)
{
    QFile file(storagePath + workspaceId + "_items.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(items).toJson());
    }
}

QJsonArray LocalStorage::loadWorkspaceItems(const QString &workspaceId) const
{
    QFile file(storagePath + workspaceId + "_items.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    return QJsonDocument::fromJson(data).array();
}

QDateTime LocalStorage::lastSyncTime() const
{
    QSettings settings(storagePath + "sync.ini", QSettings::IniFormat);
    return settings.value("lastSyncTime").toDateTime();
}

void LocalStorage::setLastSyncTime(const QDateTime &time)
{
    QSettings settings(storagePath + "sync.ini", QSettings::IniFormat);
    settings.setValue("lastSyncTime", time);
}
