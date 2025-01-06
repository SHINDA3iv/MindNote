#include "editor_widget.h"
#include "checkbox_item.h"
#include "file_item.h"
#include "image_item.h"
#include "list_item.h"
#include "nested_workspace_item.h"
#include "text_item.h"
#include "title_item.h"

#include <QFileDialog>
#include <QInputDialog>

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent),
    _layout(new QVBoxLayout(this)),
    _contextMenu(new QMenu(this))
{
    TitleItem *title = new TitleItem("");
    _layout->addWidget(title);

    _contextMenu->addAction("Добавить текст", this, &EditorWidget::onAddText);
    _contextMenu->addAction("Добавить checkbox", this, &EditorWidget::onAddCheckbox);
    _contextMenu->addAction("Добавить нумерованный список", this, &EditorWidget::onAddOrderedList);
    _contextMenu->addAction("Добавить ненумерованный список", this,
                            &EditorWidget::onAddUnorderedList);
    _contextMenu->addAction("Добавить изображение", this, &EditorWidget::onAddImage);
    _contextMenu->addAction("Добавить файл", this, &EditorWidget::onAddFile);

    setLayout(_layout);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &EditorWidget::customContextMenuRequested, this, &EditorWidget::showContextMenu);
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

        // Отображение названия текущего пространства
        setWindowTitle(_currentWorkspace->getName());
    }
}

void EditorWidget::onAddText()
{
    if (_currentWorkspace) {
        _currentWorkspace->addItem(new TextItem("Новый текст", _currentWorkspace));
    }
}

void EditorWidget::onAddCheckbox()
{
    if (_currentWorkspace) {
        _currentWorkspace->addItem(new CheckboxItem("Новый checkbox", _currentWorkspace));
    }
}

void EditorWidget::onAddOrderedList()
{
    if (_currentWorkspace) {
        ListItem *orderedList = new ListItem(ListItem::Ordered, _currentWorkspace);
        orderedList->addItemToList("Первый элемент");
        orderedList->addItemToList("Второй элемент");
        _currentWorkspace->addItem(orderedList);
    }
}

void EditorWidget::onAddUnorderedList()
{
    if (_currentWorkspace) {
        ListItem *unorderedList = new ListItem(ListItem::Unordered, _currentWorkspace);
        unorderedList->addItemToList("Первый элемент");
        unorderedList->addItemToList("Второй элемент");
        _currentWorkspace->addItem(unorderedList);
    }
}

void EditorWidget::onAddImage()
{
    QString imagePath = QFileDialog::getOpenFileName(this, "Выберите изображение", "",
                                                     "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!imagePath.isEmpty()) {
        _currentWorkspace->addItem(new ImageItem(imagePath, _currentWorkspace));
    }
}

void EditorWidget::onAddFile()
{
    if (_currentWorkspace) {
        QString filePath =
         QFileDialog::getOpenFileName(this, "Выберите файл", "", "All Files (*.*)");
        if (!filePath.isEmpty()) {
            _currentWorkspace->addItem(new FileItem(filePath, _currentWorkspace));
        }
    }
}

void EditorWidget::showContextMenu(const QPoint &pos)
{
    if (_currentWorkspace) {
        _contextMenu->exec(mapToGlobal(pos));
    }
}
