#include "SubspaceLinkItem.h"
#include "workspace.h"

SubspaceLinkItem::SubspaceLinkItem(Workspace* subspace, Workspace* parent)
    : AbstractWorkspaceItem(parent), _linkedWorkspace(subspace)
{
    if (subspace)
        _subspaceId = subspace->getId();
    _linkButton = new QPushButton(subspace ? subspace->getName() : "[Подпространство]", this);
    _linkButton->setFlat(true);
    _linkButton->setStyleSheet("color: #0078d7; text-decoration: underline; background: transparent; border: none;");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(_linkButton);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    connect(_linkButton, &QPushButton::clicked, this, [this]() {
        emit subspaceLinkClicked(_linkedWorkspace);
    });
}

QJsonObject SubspaceLinkItem::serialize() const {
    QJsonObject json;
    json["type"] = type();
    json["subspaceId"] = _subspaceId;
    return json;
}

void SubspaceLinkItem::deserialize(const QJsonObject& json) {
    if (json.contains("subspaceId")) {
        _subspaceId = json["subspaceId"].toString();
        // _linkedWorkspace будет установлен в контроллере после загрузки всех пространств
    }
}

Workspace* SubspaceLinkItem::getLinkedWorkspace() const {
    return _linkedWorkspace;
}
void SubspaceLinkItem::setLinkedWorkspace(Workspace* newLinkedWorkspace)
{
    _linkedWorkspace = newLinkedWorkspace;
}
