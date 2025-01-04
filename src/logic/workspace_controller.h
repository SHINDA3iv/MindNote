#ifndef WORKSPACE_CONTROLLER_H
#define WORKSPACE_CONTROLLER_H

#include "workspace.h"

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
    explicit WorkspaceController(QObject *parent = nullptr);

    // Создание нового рабочего пространства
    Workspace *createWorkspace(const QString &name);
    // Удаление рабочего пространства
    void removeWorkspace(Workspace *workspace);

    // Получение рабочего пространства по индексу
    Workspace *getWorkspace(int index) const;
    // Получение всех рабочих пространств
    QList<Workspace *> getAllWorkspaces() const;

    // Сериализация всех рабочих пространств в JSON
    QJsonObject serialize() const;
    // Десериализация рабочих пространств из JSON
    void deserialize(const QJsonObject &json);

    // Сохранение рабочих пространств в файл
    void saveToFile(const QString &filePath);
    // Загрузка рабочих пространств из файла
    void loadFromFile(const QString &filePath);

private:
    QList<Workspace *> workspaces;
};

#endif // WORKSPACE_CONTROLLER_H
