#ifndef LEFT_PANEL_H
#define LEFT_PANEL_H

#include "abstract_workspace_item.h"
#include "logic/workspace_controller.h"

#include <QWidget>
#include <QTreeWidget>
#include <QToolButton>

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LeftPanel(QWidget *parent = nullptr);
    ~LeftPanel() = default;

    void setWorkspaceController(WorkspaceController *controller);
    void refreshWorkspaceList();

signals:
    void workspaceSelected(Workspace *workspace);
    void subWorkspaceSelected(Workspace *workspace);

public slots:
    void onCreateWorkspace();
    void onCreateSubWorkspace();
    void updateWorkspaceList();

private slots:
    void onWorkspaceClicked(QTreeWidgetItem *item);
    void showContextMenu(const QPoint &pos);

private:
    void addSubWorkspacesToTree(QTreeWidgetItem *parentItem, Workspace *parentWorkspace);
    
    QTreeWidget *_workspaceTree;
    WorkspaceController *_workspaceController { nullptr };
};

#endif // LEFT_PANEL_H
