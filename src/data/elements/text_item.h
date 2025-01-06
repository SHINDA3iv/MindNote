#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "resizable_item.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QPointer>

class TextItem : public ResizableItem
{
    Q_OBJECT
public:
    explicit TextItem(const QString &text = "", Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QPointer<QTextEdit> _textEdit;
};

#endif // TEXTITEM_H
