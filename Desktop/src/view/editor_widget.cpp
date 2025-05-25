#include "editor_widget.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), _layout(new QVBoxLayout(this))
{
    setLayout(_layout);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);

    auto breadcrumbWidget = new QWidget(this);
    _breadcrumbLayout = new QHBoxLayout(breadcrumbWidget);
    breadcrumbWidget->setLayout(_breadcrumbLayout);
    _breadcrumbLayout->setContentsMargins(8, 2, 8, 2);
    _breadcrumbLayout->setSpacing(0);
    _layout->addWidget(breadcrumbWidget);
}

void EditorWidget::setCurrentWorkspace(Workspace *workspace)
{
    if (_currentWorkspace) {
        _layout->removeWidget(_currentWorkspace);
        _currentWorkspace->setParent(nullptr);
        _currentWorkspace = nullptr;
    }

    _currentWorkspace = workspace;
    updateBreadcrumb();
    if (_currentWorkspace) {
        _layout->addWidget(_currentWorkspace);
        _currentWorkspace->show();
        connect(_currentWorkspace, &Workspace::subWorkspaceClicked, this,
                &EditorWidget::setCurrentWorkspace);
    }
}

void EditorWidget::updateBreadcrumb()
{
    // Очищаем старые элементы
    QLayoutItem *child;
    while ((child = _breadcrumbLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    if (!_currentWorkspace)
        return;
    QList<Workspace *> chain = _currentWorkspace->getPathChain();
    int n = chain.size();
    for (int i = 0; i < n; ++i) {
        Workspace *ws = chain[i];
        QPushButton *btn = new QPushButton(ws->getName(), this);
        btn->setFlat(true);
        btn->setFixedHeight(24);
        btn->setStyleSheet(R"(
            QPushButton {
                color: #0078d7;
                text-decoration: underline;
                background: transparent;
                border: none;
                padding: 2px 4px;
                font-size: 12px;
            }
            QPushButton:hover {
                background: #f0f0f0;
            }
        )");
        connect(btn, &QPushButton::clicked, this, [this, i, chain]() {
            onBreadcrumbClicked(i);
        });
        _breadcrumbLayout->addWidget(btn);
        if (i < n - 1) {
            QLabel *sep = new QLabel("/", this);
            sep->setFixedHeight(24);
            sep->setStyleSheet("color: #666; font-size: 12px; padding: 2px 4px;");
            _breadcrumbLayout->addWidget(sep);
        }
    }
    _breadcrumbLayout->addStretch();
}

void EditorWidget::onBreadcrumbClicked(int index)
{
    if (!_currentWorkspace)
        return;
    QList<Workspace *> chain = _currentWorkspace->getPathChain();
    if (index >= 0 && index < chain.size()) {
        setCurrentWorkspace(chain[index]);
    }
}

void EditorWidget::onWorkspaceRemoved(Workspace *workspace)
{
    if (_currentWorkspace == workspace) {
        // If the current workspace is being deleted, switch to its parent
        Workspace *parent = workspace->getParentWorkspace();
        setCurrentWorkspace(parent);
    }
}

Workspace *EditorWidget::currentWorkspace() const
{
    return _currentWorkspace;
}

void EditorWidget::setGuestMode(bool isGuest)
{
    // Disable editing in guest mode
    if (_currentWorkspace) {
        _currentWorkspace->setEnabled(!isGuest);
    }
}
