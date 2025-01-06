#include "main_widget.h"

#include <QLayout>
#include <QCoreApplication>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
    _settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    initWindow();
    initConnections();

    restoreSettings();
}

MainWidget::~MainWidget()
{
    saveSettings();
}

void MainWidget::initWindow()
{
    _workspaceController = std::make_unique<WorkspaceController>();

    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    _leftPanel->setWorkspaceController(_workspaceController.get());

    _mainSplitter = new QSplitter(this);
    _mainSplitter->addWidget(_leftPanel.get());
    _mainSplitter->addWidget(_editorWidget.get());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_mainSplitter);
    setLayout(mainLayout);
}

void MainWidget::initConnections()
{
    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
}

void MainWidget::saveSettings()
{
    _settings->beginGroup("MainWidget");
    _settings->setValue("splitterState", _mainSplitter->saveState());
    _settings->endGroup();
}

void MainWidget::restoreSettings()
{
    _settings->beginGroup("MainWidget");
    QByteArray splitterState = _settings->value("splitterState").toByteArray();
    if (!splitterState.isEmpty()) {
        _mainSplitter->restoreState(splitterState);
    }
    _settings->endGroup();
}
