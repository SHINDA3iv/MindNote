#ifndef TITLEITEM_H
#define TITLEITEM_H

#include "abstract_workspace_item.h"

#include <QLabel>
#include <QVBoxLayout>

class TitleItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit TitleItem(const QString &title = "Заголовок", QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QLabel *titleLabel;
};

#endif // TITLEITEM_H
