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

    // Auth operations
    bool isAuthenticated() const;
    QString getUsername() const;
    void showAuthDialog();
    void logout();

signals:
    void authStateChanged();
    void statusMessage(const QString &message);

private slots:
    void onLogout();
    void onLoginRequested();
    void onAuthStateChanged();
    void updateAuthUI();

private:
    void initWindow();
    void initConnections();
    void restoreSettings();
    void saveSettings();
    void initApplication();

    bool _isGuestMode = true;

    QPointer<QSplitter> _mainSplitter;
    QScrollArea *_workspaceArea;
    QWidget *_sidebar;

    std::unique_ptr<WorkspaceController> _workspaceController { nullptr };

    std::shared_ptr<ApiClient> _apiClient { nullptr };
    std::shared_ptr<LocalStorage> _localStorage { nullptr };
    std::shared_ptr<SyncManager> _syncManager { nullptr };
    std::shared_ptr<AuthManager> _authManager { nullptr };

    std::unique_ptr<LeftPanel> _leftPanel { nullptr };
    std::unique_ptr<EditorWidget> _editorWidget { nullptr };

    QPointer<QSettings> _settings;

    bool _hasUnsavedChanges;
    double _zoomFactor;
    bool _sidebarVisible;
};

#endif // MAIN_WIDGET_H
