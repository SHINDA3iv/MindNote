#include "text_item.h"

TextItem::TextItem(const QString &text, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    textEdit(new QTextEdit(this))
{
    textEdit->setText(text);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    setLayout(layout);
}

QString TextItem::type() const
{
    return "TextItem";
}

QJsonObject TextItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["content"] = textEdit->toPlainText();
    return json;
}

void TextItem::deserialize(const QJsonObject &json)
{
    if (json.contains("content")) {
        textEdit->setText(json["content"].toString());
    }
}
