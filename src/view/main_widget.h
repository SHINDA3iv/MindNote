#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include "editor_widget.h"
#include "left_panel.h"

#include <QWidget>
#include <QPointer>
#include <QSplitter>
#include <QSettings>
#include <memory>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    void initWindow();
    void initConnections();
    void restoreSettings();
    void saveSettings();

    QPointer<QSplitter> _mainSplitter;

    std::unique_ptr<LeftPanel> _leftPanel { nullptr };
    std::unique_ptr<EditorWidget> _editorWidget { nullptr };
    std::unique_ptr<WorkspaceController> _workspaceController { nullptr };

    QPointer<QSettings> _settings;
};

#endif // MAIN_WIDGET_H
