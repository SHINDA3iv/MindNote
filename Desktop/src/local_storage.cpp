#include "local_storage.h"
#include <qdir.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

LocalStorage::LocalStorage(QObject *parent) : QObject(parent)
{
    storagePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/workspaces/";
    QDir().mkpath(storagePath); // Create directory if it doesn't exist
    qDebug() << "Storage path:" << storagePath;
}

void LocalStorage::saveWorkspaces(const QJsonArray &workspaces)
{
    QFile file(storagePath + "workspaces.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open workspaces file for writing:" << file.errorString();
        return;
    }
    
    QByteArray jsonData = QJsonDocument(workspaces).toJson();
    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write workspaces data:" << file.errorString();
    }
    file.close();
}

QJsonArray LocalStorage::loadWorkspaces() const
{
    QFile file(storagePath + "workspaces.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No existing workspaces file found or cannot open:" << file.errorString();
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in workspaces file";
        return QJsonArray();
    }
    return doc.array();
}

void LocalStorage::saveWorkspaceItems(const QString &workspaceId, const QJsonArray &items)
{
    QFile file(storagePath + workspaceId + "_items.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open workspace items file for writing:" << file.errorString();
        return;
    }
    
    QByteArray jsonData = QJsonDocument(items).toJson();
    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write workspace items data:" << file.errorString();
    }
    file.close();
}

QJsonArray LocalStorage::loadWorkspaceItems(const QString &workspaceId) const
{
    QFile file(storagePath + workspaceId + "_items.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No existing workspace items file found or cannot open:" << file.errorString();
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in workspace items file";
        return QJsonArray();
    }
    return doc.array();
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
    settings.sync();
}
