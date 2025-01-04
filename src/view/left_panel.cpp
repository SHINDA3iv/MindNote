#include "left_panel.h"
#include "workspaces_model.h"

#include <QTreeView>
#include <QVBoxLayout>

LeftPanel::LeftPanel(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *view = new QTreeView(this);

    auto *model = new WorkSpacesModel(this);
    model->addWorkSpace(WorkSpace("Workspace 1", "link1"));
    model->addWorkSpace(WorkSpace("Workspace 2", "link2"));

    view->setModel(model);
    view->setHeaderHidden(true);
    layout->addWidget(view);
    setLayout(layout);
}
