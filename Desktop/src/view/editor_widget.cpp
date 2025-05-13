#include "editor_widget.h"

#include <QFileDialog>
#include <QInputDialog>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), _layout(new QVBoxLayout(this))
{
    setLayout(_layout);
}

void EditorWidget::setCurrentWorkspace(Workspace *workspace)
{
    if (_currentWorkspace) {
        _layout->removeWidget(_currentWorkspace);
        _currentWorkspace->setParent(nullptr);
        _currentWorkspace = nullptr;
    }

    _currentWorkspace = workspace;

    if (_currentWorkspace) {
        // Настройка стилей для рабочего пространства
        // _currentWorkspace->setStyleSheet(R"(
        //     Workspace {
        //         background-color: white;
        //         border: 1px solid #ddd;
        //         border-radius: 4px;
        //         padding: 10px;
        //     }

        //     QLabel {
        //         margin: 5px;
        //     }

        //     QToolButton {
        //         border: none;
        //         padding: 4px;
        //         border-radius: 4px;
        //     }

        //     QToolButton:hover {
        //         background-color: #f0f0f0;
        //     }
        // )");

        _layout->addWidget(_currentWorkspace);
        _currentWorkspace->show();
    }
}
