#include "local_storage.h"
#include <QJsonDocument>
#include <QFile>
#include <qapplication.h>
#include <qbuffer.h>
#include <QDebug>

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

    qDebug() << "Saving workspace recursively:" << workspace->getName()
             << "ID:" << workspace->getId();

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
            qDebug() << "Saved icon for workspace:" << workspace->getName();
        }
    }

    // Save workspace items
    QJsonArray itemsArray;
    for (const AbstractWorkspaceItem *item : workspace->getItems()) {
        itemsArray.append(item->serialize());
        qDebug() << "Saved item of type:" << item->type()
                 << "in workspace:" << workspace->getName();
    }
    json["items"] = itemsArray;

    // Save subspaces
    QJsonArray subspaces;
    for (Workspace *sub : workspace->getSubWorkspaces()) {
        QJsonObject subJson;
        saveWorkspaceRecursive(sub, subJson);
        subspaces.append(subJson);
        qDebug() << "Saved subspace:" << sub->getName() << "for workspace:" << workspace->getName();
    }
    json["subspaces"] = subspaces;
}

Workspace *LocalStorage::loadWorkspace(const QString &workspaceName, QWidget *parent)
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

    return loadWorkspaceRecursive(doc.object(), parent);
}

Workspace *LocalStorage::loadWorkspaceRecursive(const QJsonObject &json, QWidget *parent)
{
    Workspace *workspace = new Workspace(json["name"].toString(), parent);
    workspace->setId(json["id"].toString());

    qDebug() << "Loading workspace recursively:" << workspace->getName()
             << "ID:" << workspace->getId();

    // Загрузка иконки
    if (json.contains("icon")) {
        QByteArray iconData = QByteArray::fromBase64(json["icon"].toString().toLatin1());
        QPixmap pixmap;
        if (pixmap.loadFromData(iconData)) {
            workspace->setIcon(QIcon(pixmap));
            qDebug() << "Loaded icon for workspace:" << workspace->getName();
        }
    } else {
        // Установка иконки по умолчанию если не задана пользовательская
        workspace->setIcon(QIcon(":/icons/workspace.png"));
    }

    // Load workspace items
    if (json.contains("items")) {
        QJsonArray itemsArray = json["items"].toArray();
        workspace->deserializeItems(itemsArray);
        qDebug() << "Loaded" << itemsArray.size() << "items for workspace:" << workspace->getName();
    }

    // Load subspaces
    if (json.contains("subspaces")) {
        QJsonArray subspaceArray = json["subspaces"].toArray();
        for (const QJsonValue &value : subspaceArray) {
            Workspace *sub = loadWorkspaceRecursive(value.toObject(), workspace);
            if (sub) {
                workspace->addSubWorkspace(sub);
                qDebug() << "Loaded subspace:" << sub->getName()
                         << "for workspace:" << workspace->getName();
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
