#include "checkbox_item.h"

CheckboxItem::CheckboxItem(const QString &label, bool checked, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _checkbox(new QCheckBox(label, this))
{
    _checkbox->setChecked(checked);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_checkbox);
    setLayout(layout);
    resize(width(), 50);
}

QString CheckboxItem::type() const
{
    return "CheckboxItem";
}

QJsonObject CheckboxItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["label"] = _checkbox->text();
    json["checked"] = _checkbox->isChecked();
    return json;
}

void CheckboxItem::deserialize(const QJsonObject &json)
{
    if (json.contains("label")) {
        _checkbox->setText(json["label"].toString());
    }
    if (json.contains("checked")) {
        _checkbox->setChecked(json["checked"].toBool());
    }
}
