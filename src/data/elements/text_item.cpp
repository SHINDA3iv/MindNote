#include "text_item.h"

TextItem::TextItem(const QString &text, Workspace *parent) :
    ResizableItem(parent),
    _textEdit(new QTextEdit(this))
{
    _textEdit->setText(text);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_textEdit);
    setLayout(layout);
    resize(width(), 200);
}

QString TextItem::type() const
{
    return "TextItem";
}

QJsonObject TextItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["content"] = _textEdit->toPlainText();
    return json;
}

void TextItem::deserialize(const QJsonObject &json)
{
    if (json.contains("content")) {
        _textEdit->setText(json["content"].toString());
    }
}
