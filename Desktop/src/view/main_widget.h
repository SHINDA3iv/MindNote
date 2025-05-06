#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include "../sync_manager.h"
#include "api_client.h"
#include "auth_manager.h"
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

    bool _isGuestMode = true;

    QPointer<QSplitter> _mainSplitter;

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
};

#endif // MAIN_WIDGET_H
