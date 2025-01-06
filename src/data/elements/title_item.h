#ifndef TITLEITEM_H
#define TITLEITEM_H

#include "abstract_workspace_item.h"
#include "workspace.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPointer>

class TitleItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit TitleItem(const QString &title = "Заголовок", Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QPointer<QLabel> _titleLabel;
};

#endif // TITLEITEM_H
