#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QFont>
#include <QColor>
#include <QByteArray>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    static SettingsManager& instance();

    // Theme settings
    QString theme() const;
    void setTheme(const QString& theme);
    QColor primaryColor() const;
    void setPrimaryColor(const QColor& color);
    QColor secondaryColor() const;
    void setSecondaryColor(const QColor& color);
    bool darkMode() const;
    void setDarkMode(bool enabled);

    // Editor settings
    QFont editorFont() const;
    void setEditorFont(const QFont& font);
    int tabSize() const;
    void setTabSize(int size);
    bool wordWrap() const;
    void setWordWrap(bool enabled);
    bool lineNumbers() const;
    void setLineNumbers(bool enabled);

    // Sync settings
    int syncInterval() const;
    void setSyncInterval(int minutes);
    bool autoSync() const;
    void setAutoSync(bool enabled);

    // Window settings
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);
    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    // Authentication settings
    bool rememberMe() const;
    void setRememberMe(bool enabled);
    QString lastEmail() const;
    void setLastEmail(const QString& email);

    void loadDefaults();

signals:
    void themeChanged();
    void editorSettingsChanged();
    void syncSettingsChanged();
    void windowSettingsChanged();
    void authSettingsChanged();

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() = default;
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings _settings;
};

#endif // SETTINGS_MANAGER_H 