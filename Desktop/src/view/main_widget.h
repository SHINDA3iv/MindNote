#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include "../sync_manager.h"
#include "api/api_client.h"
#include "api/auth_manager.h"
#include "editor_widget.h"
#include "left_panel.h"

#include <QWidget>
#include <QPointer>
#include <QSplitter>
#include <QSettings>
#include <memory>
#include <QScrollArea>
#include <QToolBar>
#include <QStatusBar>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    // Workspace operations
    void createNewWorkspace();
    void openWorkspace();
    bool saveCurrentWorkspace();
    bool saveWorkspaceAs();
    void syncWorkspaces();

    // Edit operations
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

    // View operations
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleSidebar();

    // State
    bool hasUnsavedChanges() const;

private slots:
    void onLogout();
    void onLoginRequested();

private:
    void initWindow();
    void initConnections();
    void restoreSettings();
    void saveSettings();
    void showAuthDialog();
    void initApplication();
    void initUI();
    void createToolbars();
    void createStatusBar();

    bool _isGuestMode = true;

    QPointer<QSplitter> _mainSplitter;
    QScrollArea* _workspaceArea;
    QWidget* _sidebar;
    QToolBar* _mainToolBar;
    QToolBar* _editToolBar;
    QToolBar* _viewToolBar;
    QStatusBar* _statusBar;

    std::unique_ptr<WorkspaceController> _workspaceController { nullptr };

    std::shared_ptr<ApiClient> _apiClient { nullptr };
    std::shared_ptr<LocalStorage> _localStorage { nullptr };
    std::shared_ptr<SyncManager> _syncManager { nullptr };
    std::shared_ptr<AuthManager> _authManager { nullptr };

    std::unique_ptr<LeftPanel> _leftPanel { nullptr };
    std::unique_ptr<EditorWidget> _editorWidget { nullptr };

    QPointer<QToolButton> _logoutButton { nullptr };
    QPointer<QToolButton> _loginButton { nullptr };
    QAction *_syncAction { nullptr };

    QPointer<QSettings> _settings;

    bool _hasUnsavedChanges;
    double _zoomFactor;
    bool _sidebarVisible;
};

#endif // MAIN_WIDGET_H
