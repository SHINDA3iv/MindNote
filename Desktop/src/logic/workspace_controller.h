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
                                 QObject *parent = nullptr);

    // Создание нового рабочего пространства
    Workspace *createWorkspace(const QString &name, const QString &id = QString());
    // Удаление рабочего пространства
    void removeWorkspace(Workspace *workspace);

    // Получение рабочего пространства по индексу
    Workspace *getWorkspace(int index) const;
    // Получение всех рабочих пространств
    QList<Workspace *> getAllWorkspaces() const;

    Workspace *getWorkspaceById(const QString &id) const;

    // Сериализация всех рабочих пространств в JSON
    QJsonObject serialize() const;
    // Десериализация рабочих пространств из JSON
    void deserialize(const QJsonObject &json);

    // Сохранение рабочих пространств в файл
    void saveToFile(const QString &filePath);
    // Загрузка рабочих пространств из файла
    void loadFromFile(const QString &filePath);

    // Сохранение всех рабочих пространств
    void saveWorkspaces();
    // Загрузка рабочих пространств
    void loadWorkspaces();

    // --- ВЛОЖЕННОСТЬ ---
    Workspace* createSubWorkspace(Workspace* parent, const QString& name, const QString& id = QString());
    QList<Workspace*> getRootWorkspaces() const;
    Workspace* findWorkspaceById(const QString& id) const;

private:
    QList<Workspace *> _workspaces;
    std::shared_ptr<LocalStorage> _localStorage;
};

#endif // WORKSPACE_CONTROLLER_H
