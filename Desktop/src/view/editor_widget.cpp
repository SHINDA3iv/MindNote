#include "editor_widget.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), _layout(new QVBoxLayout(this))
{
    setLayout(_layout);
    _breadcrumbLayout = new QHBoxLayout();
    _layout->addLayout(_breadcrumbLayout);
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
        connect(_currentWorkspace, &Workspace::subWorkspaceClicked, this, &EditorWidget::setCurrentWorkspace);
    }
}

void EditorWidget::updateBreadcrumb()
{
    // Очищаем старые элементы
    QLayoutItem* child;
    while ((child = _breadcrumbLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    if (!_currentWorkspace) return;
    QList<Workspace*> chain = _currentWorkspace->getPathChain();
    int n = chain.size();
    for (int i = 0; i < n; ++i) {
        Workspace* ws = chain[i];
        QPushButton* btn = new QPushButton(ws->getName(), this);
        btn->setFlat(true);
        btn->setStyleSheet("color: #0078d7; text-decoration: underline; background: transparent; border: none;");
        connect(btn, &QPushButton::clicked, this, [this, i, chain]() { onBreadcrumbClicked(i); });
        _breadcrumbLayout->addWidget(btn);
        if (i < n - 1) {
            QLabel* sep = new QLabel("/", this);
            _breadcrumbLayout->addWidget(sep);
        }
    }
    _breadcrumbLayout->addStretch();
}

void EditorWidget::onBreadcrumbClicked(int index)
{
    if (!_currentWorkspace) return;
    QList<Workspace*> chain = _currentWorkspace->getPathChain();
    if (index >= 0 && index < chain.size()) {
        setCurrentWorkspace(chain[index]);
    }
}
