#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "abstract_workspace_item.h"

#include <QTextEdit>
#include <QVBoxLayout>

class TextItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit TextItem(const QString &text = "", QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QTextEdit *textEdit;
};

#endif // TEXTITEM_H
