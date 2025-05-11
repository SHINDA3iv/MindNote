#ifndef LEFT_PANEL_H
#define LEFT_PANEL_H

#include "abstract_workspace_item.h"
#include "logic/workspace_controller.h"

#include <QWidget>
#include <QListWidget>
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

private slots:
    void onWorkspaceClicked(QListWidgetItem *item);
    void onCreateWorkspace();

    void showContextMenu(const QPoint &pos);

private:
    QListWidget *_workspaceList;
    QToolButton *_createWorkspaceButton;
    WorkspaceController *_workspaceController { nullptr };
};

#endif // LEFT_PANEL_H
