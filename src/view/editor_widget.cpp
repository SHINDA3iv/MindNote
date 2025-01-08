#include "editor_widget.h"
#include "title_item.h"

#include <QFileDialog>
#include <QInputDialog>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), _layout(new QVBoxLayout(this))
{
    TitleItem *title = new TitleItem("");
    _layout->addWidget(title);

    setLayout(_layout);
}

void EditorWidget::setCurrentWorkspace(Workspace *workspace)
{
    if (_currentWorkspace) {
        _layout->removeWidget(_currentWorkspace);
        _currentWorkspace->setParent(nullptr);
    }

    _currentWorkspace = workspace;

    if (_currentWorkspace) {
        _layout->addWidget(_currentWorkspace);
        _currentWorkspace->show();

        setWindowTitle(_currentWorkspace->getName());
    }
}
