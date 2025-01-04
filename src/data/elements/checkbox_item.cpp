#include "checkbox_item.h"

CheckboxItem::CheckboxItem(const QString &label, bool checked, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    checkbox(new QCheckBox(label, this))
{
    checkbox->setChecked(checked);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(checkbox);
    setLayout(layout);
}

QString CheckboxItem::type() const
{
    return "CheckboxItem";
}

QJsonObject CheckboxItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["label"] = checkbox->text();
    json["checked"] = checkbox->isChecked();
    return json;
}

void CheckboxItem::deserialize(const QJsonObject &json)
{
    if (json.contains("label")) {
        checkbox->setText(json["label"].toString());
    }
    if (json.contains("checked")) {
        checkbox->setChecked(json["checked"].toBool());
    }
}
