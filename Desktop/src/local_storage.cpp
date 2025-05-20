#include "local_storage.h"
#include <QJsonDocument>
#include <QFile>
#include <qapplication.h>
#include <qbuffer.h>

LocalStorage::LocalStorage(QObject *parent) : QObject(parent)
{
    storagePath = QApplication::applicationDirPath() + "/Workspaces/";
    QDir().mkpath(storagePath);
}

void LocalStorage::saveWorkspace(Workspace *workspace)
{
    if (!workspace)
        return;

    QString workspacePath = storagePath + workspace->getName() + "/";
    QDir().mkpath(workspacePath);

    QFile file(workspacePath + "workspace.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open workspace file for writing:" << file.errorString();
        return;
    }

    QJsonObject json;
    saveWorkspaceRecursive(workspace, json);
    QByteArray jsonData = QJsonDocument(json).toJson();

    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write workspace data:" << file.errorString();
    }
    file.close();
}

void LocalStorage::saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json)
{
    json["name"] = workspace->getName();
    json["id"] = workspace->getId();

    // Сохранение иконки
    QIcon icon = workspace->getIcon();
    if (!icon.isNull()) {
        QByteArray iconData;
        QBuffer buffer(&iconData);
        buffer.open(QIODevice::WriteOnly);

        // Сохраняем первую доступную pixmap
        QPixmap pixmap = icon.pixmap(icon.availableSizes().first());
        if (pixmap.save(&buffer, "PNG")) {
            json["icon"] = QString::fromLatin1(iconData.toBase64().data());
        }
    }

    QJsonArray subspaces;
    for (Workspace *sub : workspace->getSubWorkspaces()) {
        QJsonObject subJson;
        saveWorkspaceRecursive(sub, subJson);
        subspaces.append(subJson);
    }
    json["subspaces"] = subspaces;
}

Workspace *LocalStorage::loadWorkspace(const QString &workspaceName)
{
    QString workspacePath = storagePath + workspaceName + "/workspace.json";
    QFile file(workspacePath);

    if (!file.exists()) {
        qWarning() << "Workspace file not found:" << workspacePath;
        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open workspace file for reading:" << file.errorString();
        return nullptr;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in workspace file:" << workspacePath;
        return nullptr;
    }

    return loadWorkspaceRecursive(doc.object());
}

Workspace *LocalStorage::loadWorkspaceRecursive(const QJsonObject &json, Workspace *parent)
{
    Workspace *workspace = new Workspace(json["name"].toString(), parent);
    workspace->setId(json["id"].toString());

    // Загрузка иконки
    if (json.contains("icon")) {
        QByteArray iconData = QByteArray::fromBase64(json["icon"].toString().toLatin1());
        QPixmap pixmap;
        if (pixmap.loadFromData(iconData)) {
            workspace->setIcon(QIcon(pixmap));
        }
    } else {
        // Установка иконки по умолчанию если не задана пользовательская
        workspace->setIcon(QIcon(":/icons/workspace.png"));
    }

    if (json.contains("subspaces")) {
        QJsonArray subspaceArray = json["subspaces"].toArray();
        for (const QJsonValue &value : subspaceArray) {
            Workspace *sub = loadWorkspaceRecursive(value.toObject(), workspace);
            if (sub) {
                workspace->addSubWorkspace(sub);
            }
        }
    }

    return workspace;
}

void LocalStorage::deleteWorkspace(const QString &workspaceName)
{
    QString workspacePath = storagePath + workspaceName + "/";
    QDir dir(workspacePath);

    if (dir.exists()) {
        // Удаляем все содержимое папки
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &file : files) {
            if (file.isDir()) {
                deleteWorkspace(file.baseName());
            } else {
                QFile::remove(file.absoluteFilePath());
            }
        }

        // Удаляем саму папку
        dir.rmdir(workspacePath);
    }
}
