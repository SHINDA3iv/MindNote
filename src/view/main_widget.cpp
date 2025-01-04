#include "main_widget.h"

#include <QLayout>
#include <QPointer>
#include <QSplitter>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    initWindow();
    initConnections();
}

void MainWidget::initWindow()
{
    _workspaceController = std::make_unique<WorkspaceController>();

    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    _leftPanel->setWorkspaceController(_workspaceController.get());

    QSplitter *mainSplitter = new QSplitter(this);
    mainSplitter->addWidget(_leftPanel.get());
    mainSplitter->addWidget(_editorWidget.get());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainSplitter);
    setLayout(mainLayout);
}

void MainWidget::initConnections()
{
    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
}
