#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include "abstract_workspace_item.h"

#include <QCheckBox>
#include <QVBoxLayout>

class CheckboxItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit CheckboxItem(const QString &label = "Задача",
                          bool checked = false,
                          QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;

    void deserialize(const QJsonObject &json) override;

private:
    QCheckBox *checkbox;
};

#endif // CHECKBOXITEM_H
