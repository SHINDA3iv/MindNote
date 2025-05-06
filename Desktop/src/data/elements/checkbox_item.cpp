#include "checkbox_item.h"
#include <qevent.h>
#include <qmenu.h>

CheckboxItem::CheckboxItem(const QString &label, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _checkbox(new QCheckBox(label, this)),
    _editLine(new QLineEdit(this))
{
    _editLine->setVisible(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_checkbox);
    layout->addWidget(_editLine);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    resize(width(), 25);

    connect(_editLine, &QLineEdit::editingFinished, this, &CheckboxItem::finishEditing);

    // _checkbox->setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(_checkbox, &QWidget::customContextMenuRequested, this,
    // &CheckboxItem::showContextMenu);
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

void CheckboxItem::startEditing()
{
    _editLine->setText(_checkbox->text());
    _editLine->setVisible(true);
    _checkbox->setVisible(false);
    _editLine->setFocus();
}

void CheckboxItem::finishEditing()
{
    _checkbox->setText(_editLine->text());
    _editLine->setVisible(false);
    _checkbox->setVisible(true);
}

void CheckboxItem::addCustomContextMenuActions(QMenu *contextMenu)
{
    QAction *editAction = contextMenu->addAction("Изменить текст");
    connect(editAction, &QAction::triggered, this, &CheckboxItem::startEditing);
}
