#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include "workspace.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget() = default;

    void setCurrentWorkspace(Workspace *workspace);

private slots:
    void onAddText();
    void onAddCheckbox();
    void onAddOrderedList();
    void onAddUnorderedList();
    void onAddImage();
    void onAddFile();
    void showContextMenu(const QPoint &pos);

private:
    Workspace *_currentWorkspace { nullptr };
    QVBoxLayout *_layout;
    QMenu *_contextMenu;
};

#endif // EDITOR_WIDGET_H
