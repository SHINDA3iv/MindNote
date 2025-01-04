#include "editor_widget.h"

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent),
    layout(new QVBoxLayout(this)),
    addTitleButton(new QPushButton("Добавить заголовок", this)),
    addTextButton(new QPushButton("Добавить текст", this)),
    addCheckboxButton(new QPushButton("Добавить checkbox", this)),
    addImageButton(new QPushButton("Добавить изображение", this)),
    addFileButton(new QPushButton("Добавить файл", this))
{
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(addTitleButton);
    buttonLayout->addWidget(addTextButton);
    buttonLayout->addWidget(addCheckboxButton);
    buttonLayout->addWidget(addImageButton);
    buttonLayout->addWidget(addFileButton);

    layout->addLayout(buttonLayout);
    setLayout(layout);

    connect(addTitleButton, &QPushButton::clicked, this, &EditorWidget::onAddTitle);
    connect(addTextButton, &QPushButton::clicked, this, &EditorWidget::onAddText);
    connect(addCheckboxButton, &QPushButton::clicked, this, &EditorWidget::onAddCheckbox);
    connect(addImageButton, &QPushButton::clicked, this, &EditorWidget::onAddImage);
    connect(addFileButton, &QPushButton::clicked, this, &EditorWidget::onAddFile);
}

void EditorWidget::setCurrentWorkspace(Workspace *workspace)
{
    if (currentWorkspace) {
        layout->removeWidget(currentWorkspace);
        currentWorkspace->setParent(nullptr);
    }

    currentWorkspace = workspace;

    if (currentWorkspace) {
        layout->addWidget(currentWorkspace);
        currentWorkspace->show();
    }
}

void EditorWidget::onAddTitle()
{
    if (currentWorkspace) {
        currentWorkspace->addItem(new TitleItem("Новый заголовок"));
    }
}

void EditorWidget::onAddText()
{
    if (currentWorkspace) {
        currentWorkspace->addItem(new TextItem("Новый текст"));
    }
}

void EditorWidget::onAddCheckbox()
{
    if (currentWorkspace) {
        currentWorkspace->addItem(new CheckboxItem("Новый checkbox"));
    }
}

void EditorWidget::onAddImage()
{
    if (currentWorkspace) {
        currentWorkspace->addItem(new ImageItem());
    }
}

void EditorWidget::onAddFile()
{
    if (currentWorkspace) {
        currentWorkspace->addItem(new FileItem());
    }
}
