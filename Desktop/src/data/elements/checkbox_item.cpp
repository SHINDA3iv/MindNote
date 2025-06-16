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
    resize(width(), 35);

    _editLine->setStyleSheet(R"(
        QLineEdit {
            padding: 0px;
        }
    )");

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
    _editLine->setFixedHeight(30); // Ensure fixed height when editing
    _editLine->setFocus();
    _editLine->selectAll(); // Select all text for easier editing
}

void CheckboxItem::finishEditing()
{
    _checkbox->setText(_editLine->text());
    _editLine->setVisible(false);
    _checkbox->setVisible(true);
    _checkbox->setFixedHeight(30); // Match checkbox height with line edit
}

void CheckboxItem::addCustomContextMenuActions(QMenu *contextMenu)
{
    QAction *editAction = contextMenu->addAction("Изменить текст");
    connect(editAction, &QAction::triggered, this, &CheckboxItem::startEditing);
}
