#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "Workspace.h"

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget() = default;

    void setCurrentWorkspace(Workspace *workspace);

private slots:
    void onAddTitle();
    void onAddText();
    void onAddCheckbox();
    void onAddImage();
    void onAddFile();

private:
    Workspace *currentWorkspace { nullptr };
    QVBoxLayout *layout;
    QPushButton *addTitleButton;
    QPushButton *addTextButton;
    QPushButton *addCheckboxButton;
    QPushButton *addImageButton;
    QPushButton *addFileButton;
};

#endif // EDITOR_WIDGET_H
