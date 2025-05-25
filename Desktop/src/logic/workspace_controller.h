#ifndef WORKSPACE_CONTROLLER_H
#define WORKSPACE_CONTROLLER_H

#include "workspace.h"
#include "../local_storage.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

class WorkspaceController : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceController(std::shared_ptr<LocalStorage> localStorage,
                                 QWidget *parent = nullptr);

    Workspace *createWorkspace(const QString &title);
    void removeWorkspace(Workspace *workspace);
    Workspace *getWorkspaceByTitle(const QString &title) const;
    QList<Workspace *> getAllWorkspaces() const;

    // Сериализация и сохранение данных
    QJsonObject serialize() const;
    void deserialize(const QJsonObject &json);
    void saveWorkspaces();
    void loadWorkspaces(QWidget *parent = nullptr);

    // Работа с подпространствами (страницами)
    Workspace *createSubWorkspace(Workspace *parent, const QString &title);
    QList<Workspace *> getRootWorkspaces() const;

public:
    // Получение рабочего пространства по индексу
    Workspace *getWorkspace(int index) const;

    // Сохранение рабочих пространств в файл
    void saveToFile(const QString &filePath);
    // Загрузка рабочих пространств из файла
    void loadFromFile(const QString &filePath);

    Workspace *findWorkspaceByTitle(const QString &title) const;

    Workspace *findWorkspaceRecursive(Workspace *workspace, const QString &title) const;
signals:
    void workspaceAdded(Workspace *workspace);
    void workspaceRemoved(Workspace *workspace);
    void pathUpdated(Workspace *workspace);

private slots:
    void handleAddSubspaceRequest(Workspace *parent);

private:
    QList<Workspace *> _workspaces;
    std::shared_ptr<LocalStorage> _localStorage;

    void recursiveSerialize(Workspace *workspace, QJsonObject &json) const;
    Workspace *recursiveDeserialize(const QJsonObject &json, Workspace *parent = nullptr);
};

#endif // WORKSPACE_CONTROLLER_H
