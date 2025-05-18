#ifndef SUBSPACELINKITEM_H
#define SUBSPACELINKITEM_H

#include "abstract_workspace_item.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>

class Workspace;

class SubspaceLinkItem : public AbstractWorkspaceItem {
    Q_OBJECT
public:
    explicit SubspaceLinkItem(Workspace* subspace, Workspace* parent = nullptr);
    QString type() const override { return "SubspaceLinkItem"; }
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& json) override;
    Workspace* getLinkedWorkspace() const;
    void setLinkedWorkspace(Workspace* newLinkedWorkspace);

signals:
    void subspaceLinkClicked(Workspace* subspace);

private:
    QPointer<QPushButton> _linkButton;
    QString _subspaceId;
    Workspace* _linkedWorkspace { nullptr };
};

#endif // SUBSPACELINKITEM_H 
