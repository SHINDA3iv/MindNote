#include "title_item.h"

TitleItem::TitleItem(const QString &title, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    titleLabel(new QLabel(title, this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);
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
    json["content"] = titleLabel->text();
    return json;
}

void TitleItem::deserialize(const QJsonObject &json)
{
    if (json.contains("content")) {
        titleLabel->setText(json["content"].toString());
    }
}
