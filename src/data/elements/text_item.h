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

private slots:
    void toggleBold();
    void toggleItalic();
    // void insertList();

    void insertOrderedList();
    void insertUnorderedList();

private:
    void mergeCurrentCharFormat(const QTextCharFormat &format);
    QPointer<QTextEdit> _textEdit;
};

#endif // TEXTITEM_H
