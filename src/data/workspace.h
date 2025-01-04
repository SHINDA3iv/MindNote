#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QList>
#include "abstract_workspace_item.h"
#include "checkbox_item.h"
#include "file_item.h"
#include "image_item.h"
#include "list_item.h"
#include "nested_workspace_item.h"
#include "text_item.h"
#include "title_item.h"
#include <qjsonarray.h>

class Workspace : public QWidget
{
    Q_OBJECT

public:
    explicit Workspace(const QString &name = "Новое пространство", QWidget *parent = nullptr);

    QString getName() const;
    void setName(const QString &name);

    void addItem(AbstractWorkspaceItem *item);
    void removeItem(AbstractWorkspaceItem *item);

    QJsonObject serialize() const;
    void deserialize(const QJsonObject &json);

private:
    QString workspaceName;
    QVBoxLayout *layout;
    QList<AbstractWorkspaceItem *> items;
};

#endif // WORKSPACE_H
