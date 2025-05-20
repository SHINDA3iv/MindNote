#include "SubspaceLinkItem.h"
#include "workspace.h"
#include <QHBoxLayout>
#include <QLabel>

SubspaceLinkItem::SubspaceLinkItem(Workspace *subspace, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _linkedWorkspace(subspace)
{
    if (subspace)
        _subspaceId = subspace->getId();

    // Create horizontal layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignTop);

    // Create icon container
    QWidget *iconContainer = new QWidget(this);
    QVBoxLayout *iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(8, 8, 8, 8);
    iconLayout->setSpacing(0);

    // Create icon label
    QLabel *iconLabel = new QLabel(iconContainer);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setMinimumSize(40, 40);
    iconLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QPixmap iconPixmap;
    if (!subspace->getIcon().isNull()) {
        iconPixmap = subspace->getIcon().pixmap(40, 40);
    } else {
        iconPixmap = QIcon(":/icons/workspace.png").pixmap(40, 40);
    }
    iconLabel->setPixmap(iconPixmap);
    iconLayout->addWidget(iconLabel, 0, Qt::AlignCenter);

    layout->addWidget(iconContainer, 0, Qt::AlignTop);

    // Create button
    _linkButton = new QPushButton(subspace ? subspace->getName() : "[Подпространство]", this);
    _linkButton->setFlat(true);
    _linkButton->setStyleSheet(R"(
        QPushButton {
            color: #0078d7;
            text-decoration: underline;
            background: transparent;
            border: none;
            text-align: left;
            padding: 0;
            min-height: 24px;
            font-size: 15px;
        }
        QPushButton:hover {
            background: #f0f0f0;
            border-radius: 4px;
        }
    )");
    _linkButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _linkButton->setFixedHeight(24);
    layout->addWidget(_linkButton, 1, Qt::AlignTop);

    // Set fixed height for the entire item
    setFixedHeight(80);

    setLayout(layout);
    connect(_linkButton, &QPushButton::clicked, this, [this]() {
        emit subspaceLinkClicked(_linkedWorkspace);
    });
}

QJsonObject SubspaceLinkItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["subspaceId"] = _linkedWorkspace ? _linkedWorkspace->getId() : _subspaceId;
    json["subspaceName"] = _linkedWorkspace ? _linkedWorkspace->getName() : "[Подпространство]";
    return json;
}

void SubspaceLinkItem::deserialize(const QJsonObject &json)
{
    if (json.contains("subspaceId")) {
        _subspaceId = json["subspaceId"].toString();
        QString name =
         json.contains("subspaceName") ? json["subspaceName"].toString() : "[Подпространство]";
        _linkButton->setText(name);
    }
}

Workspace *SubspaceLinkItem::getLinkedWorkspace() const
{
    return _linkedWorkspace;
}

void SubspaceLinkItem::setLinkedWorkspace(Workspace *newLinkedWorkspace)
{
    _linkedWorkspace = newLinkedWorkspace;
    if (_linkedWorkspace) {
        _subspaceId = _linkedWorkspace->getId();
        _linkButton->setText(_linkedWorkspace->getName());

        // Update icon
        QLabel *iconLabel =
         qobject_cast<QLabel *>(layout()->itemAt(0)->widget()->layout()->itemAt(0)->widget());
        if (iconLabel) {
            if (!_linkedWorkspace->getIcon().isNull()) {
                iconLabel->setPixmap(
                 _linkedWorkspace->getIcon().pixmap(QSize(40, 40), QIcon::Normal, QIcon::On));
            } else {
                iconLabel->setPixmap(QIcon(":/icons/workspace.png").pixmap(40, 40));
            }
        }
    }
}

void SubspaceLinkItem::deleteItem()
{
    if (_linkedWorkspace) {
        Workspace *parent = qobject_cast<Workspace *>(this->parent());
        if (parent) {
            parent->removeSubWorkspace(_linkedWorkspace);
        }
    }
    AbstractWorkspaceItem::deleteItem();
}
