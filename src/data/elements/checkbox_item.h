#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include "abstract_workspace_item.h"
#include "workspace.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QPointer>

class CheckboxItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit CheckboxItem(const QString &label = "Задача",
                          bool checked = false,
                          Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QPointer<QCheckBox> _checkbox;
};

#endif // CHECKBOXITEM_H
