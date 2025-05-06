#include "title_item.h"

TitleItem::TitleItem(const QString &title, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _titleLabel(new QLabel(title, this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_titleLabel);
    setLayout(layout);
}

QString TitleItem::type() const
{
    return "TitleItem";
}

QJsonObject TitleItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["content"] = _titleLabel->text();
    return json;
}

void TitleItem::deserialize(const QJsonObject &json)
{
    if (json.contains("content")) {
        _titleLabel->setText(json["content"].toString());
    }
}
