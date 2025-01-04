#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include "editor_widget.h"
#include "left_panel.h"

#include <QWidget>
#include <memory>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget() = default;

private:
    void initWindow();
    void initConnections();

    std::unique_ptr<LeftPanel> _leftPanel { nullptr };
    std::unique_ptr<EditorWidget> _editorWidget { nullptr };
    std::unique_ptr<WorkspaceController> _workspaceController { nullptr };
};

#endif // MAIN_WIDGET_H
