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

private:
    Workspace *_currentWorkspace { nullptr };
    QVBoxLayout *_layout;
};

#endif // EDITOR_WIDGET_H
