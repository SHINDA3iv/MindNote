#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "abstract_workspace_item.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QList>
#include <QJsonArray>
#include <QPointer>
#include <QLabel>
#include <QScrollArea>

class Workspace : public QWidget
{
    Q_OBJECT

public:
    explicit Workspace(const QString &title = "Новое пространство", QWidget *parent = nullptr);

    QString getTitle() const;
    void setTitle(const QString &title);

    void addItem(AbstractWorkspaceItem *item);
    void removeItem(AbstractWorkspaceItem *item);

    QJsonObject serialize() const;
    void deserialize(const QJsonObject &json);

    void deserializeItems(const QJsonArray &itemsArray);

    QList<AbstractWorkspaceItem *> getItems() const;

    void addItemByType(const QString &type);

    void setIcon(const QIcon &icon);
    QLabel *getIconLabel();
    QIcon getIcon() const;

    // --- ВЛОЖЕННОСТЬ ---
    Workspace *getParentWorkspace() const;
    void setParentWorkspace(Workspace *parent);
    QList<Workspace *> getSubWorkspaces() const;
    void addSubWorkspace(Workspace *sub);
    void removeSubWorkspace(Workspace *sub);
    bool hasSubWorkspaceWithTitle(const QString &title) const;
    QString getFullPathTitle() const;
    QList<Workspace *> getPathChain() const;

    QString getPath() const;

    // Version management
    QString getVersion() const;
    void setVersion(const QString &version);

    // Owner management
    QString getOwner() const;
    void setOwner(const QString &owner);

public slots:
    Workspace *getRootWorkspace();
signals:
    void subWorkspaceClicked(Workspace *subspace);
    void addSubspaceRequested();

private:
    void updateContentSize();

    QString _title;
    QString _version;
    QString _owner;

    QPointer<QVBoxLayout> _layout;
    QPointer<QScrollArea> _scrollArea;
    QPointer<QWidget> _contentWidget;
    QPointer<QLabel> _titleLabel;
    QPointer<QLabel> _iconLabel;
    QIcon _icon;

    QList<AbstractWorkspaceItem *> _items;
    QSpacerItem *_spacerItem { nullptr };

    Workspace *_parentWorkspace { nullptr };
    QList<Workspace *> _subWorkspaces;
};

#endif // WORKSPACE_H
