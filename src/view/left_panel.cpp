#include "left_panel.h"

#include <QInputDialog>
#include <QMenu>

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

    _workspaceList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_workspaceList, &QListWidget::customContextMenuRequested, this,
            &LeftPanel::showContextMenu);
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

    bool ok;
    QString workspaceName = QInputDialog::getText(this, tr("Создание пространства"),
                                                  tr("Введите имя нового пространства:"),
                                                  QLineEdit::Normal, tr("Новое пространство"), &ok);

    if (ok && !workspaceName.isEmpty()) {
        Workspace *newWorkspace = _workspaceController->createWorkspace(workspaceName);
        refreshWorkspaceList();
        emit workspaceSelected(newWorkspace);
    }
}

void LeftPanel::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = _workspaceList->itemAt(pos);
    if (!item || !_workspaceController)
        return;

    Workspace *workspace = static_cast<Workspace *>(item->data(Qt::UserRole).value<void *>());
    if (!workspace)
        return;

    QMenu contextMenu(this);

    QAction *renameAction = contextMenu.addAction("Сменить название");
    connect(renameAction, &QAction::triggered, [this, workspace, item]() {
        bool ok;
        QString newName =
         QInputDialog::getText(this, "Сменить название пространства",
                               "Новое название:", QLineEdit::Normal, workspace->getName(), &ok);
        if (ok && !newName.isEmpty()) {
            workspace->setName(newName);
            item->setText(newName);
        }
    });

    QAction *deleteAction = contextMenu.addAction("Удалить пространство");
    connect(deleteAction, &QAction::triggered, [this, workspace]() {
        _workspaceController->removeWorkspace(workspace);
        refreshWorkspaceList();
    });

    contextMenu.exec(_workspaceList->viewport()->mapToGlobal(pos));
}
