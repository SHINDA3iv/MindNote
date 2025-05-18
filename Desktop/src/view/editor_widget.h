#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include "workspace.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget() = default;

    void setCurrentWorkspace(Workspace *workspace);

public slots:
    void onWorkspaceRemoved(Workspace* workspace);

private:
    Workspace *_currentWorkspace { nullptr };
    QVBoxLayout *_layout;
    QHBoxLayout *_breadcrumbLayout { nullptr };
    QLabel *_breadcrumbLabel { nullptr };
    void updateBreadcrumb();

private slots:
    void onBreadcrumbClicked(int index);
};

#endif // EDITOR_WIDGET_H
