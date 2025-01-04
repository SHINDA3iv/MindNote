#include "left_panel.h"

LeftPanel::LeftPanel(QWidget *parent) :
    QWidget(parent),
    _workspaceList(new QListWidget(this)),
    _createWorkspaceButton(new QPushButton("Создать пространство", this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_workspaceList);
    layout->addWidget(_createWorkspaceButton);
    setLayout(layout);

    connect(_workspaceList, &QListWidget::itemClicked, this, &LeftPanel::onWorkspaceClicked);
    connect(_createWorkspaceButton, &QPushButton::clicked, this, &LeftPanel::onCreateWorkspace);
}

void LeftPanel::setWorkspaceController(WorkspaceController *controller)
{
    _workspaceController = controller;
    refreshWorkspaceList();
}

void LeftPanel::refreshWorkspaceList()
{
    if (!_workspaceController)
        return;

    _workspaceList->clear();

    auto workspaces = _workspaceController->getAllWorkspaces();
    for (Workspace *workspace : workspaces) {
        QListWidgetItem *item = new QListWidgetItem(workspace->getName(), _workspaceList);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void *>(workspace)));
    }
}

void LeftPanel::onWorkspaceClicked(QListWidgetItem *item)
{
    if (!item || !_workspaceController)
        return;

    Workspace *workspace = static_cast<Workspace *>(item->data(Qt::UserRole).value<void *>());
    if (workspace) {
        emit workspaceSelected(workspace);
    }
}

void LeftPanel::onCreateWorkspace()
{
    if (!_workspaceController)
        return;

    Workspace *newWorkspace = _workspaceController->createWorkspace("Новое пространство");
    refreshWorkspaceList();
}
