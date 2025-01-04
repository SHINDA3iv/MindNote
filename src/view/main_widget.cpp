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
    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    QSplitter *mainSplitter = new QSplitter(this);
    mainSplitter->addWidget(_leftPanel.get());
    mainSplitter->addWidget(_editorWidget.get());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainSplitter);
    setLayout(mainLayout);
}

void MainWidget::initConnections()
{}
