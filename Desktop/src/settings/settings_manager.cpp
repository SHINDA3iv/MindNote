#include "settings_manager.h"
#include <QApplication>
#include <QFontDatabase>

SettingsManager& SettingsManager::instance()
{
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , _settings("MindNote", "Desktop")
{
    loadDefaults();
}

void SettingsManager::loadDefaults()
{
    // Theme defaults
    if (!_settings.contains("theme/name")) {
        setTheme("light");
    }
    if (!_settings.contains("theme/primaryColor")) {
        setPrimaryColor(QColor("#1976D2"));
    }
    if (!_settings.contains("theme/secondaryColor")) {
        setSecondaryColor(QColor("#FFA000"));
    }
    if (!_settings.contains("theme/darkMode")) {
        setDarkMode(false);
    }

    // Editor defaults
    if (!_settings.contains("editor/font")) {
        QFont defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        defaultFont.setPointSize(10);
        setEditorFont(defaultFont);
    }
    if (!_settings.contains("editor/tabSize")) {
        setTabSize(4);
    }
    if (!_settings.contains("editor/wordWrap")) {
        setWordWrap(true);
    }
    if (!_settings.contains("editor/lineNumbers")) {
        setLineNumbers(true);
    }

    // Sync defaults
    if (!_settings.contains("sync/interval")) {
        setSyncInterval(5);
    }
    if (!_settings.contains("sync/autoSync")) {
        setAutoSync(true);
    }

    // Auth defaults
    if (!_settings.contains("auth/rememberMe")) {
        setRememberMe(false);
    }
}

// Theme settings
QString SettingsManager::theme() const
{
    return _settings.value("theme/name").toString();
}

void SettingsManager::setTheme(const QString& theme)
{
    _settings.setValue("theme/name", theme);
    emit themeChanged();
}

QColor SettingsManager::primaryColor() const
{
    return _settings.value("theme/primaryColor").value<QColor>();
}

void SettingsManager::setPrimaryColor(const QColor& color)
{
    _settings.setValue("theme/primaryColor", color);
    emit themeChanged();
}

QColor SettingsManager::secondaryColor() const
{
    return _settings.value("theme/secondaryColor").value<QColor>();
}

void SettingsManager::setSecondaryColor(const QColor& color)
{
    _settings.setValue("theme/secondaryColor", color);
    emit themeChanged();
}

bool SettingsManager::darkMode() const
{
    return _settings.value("theme/darkMode").toBool();
}

void SettingsManager::setDarkMode(bool enabled)
{
    _settings.setValue("theme/darkMode", enabled);
    emit themeChanged();
}

// Editor settings
QFont SettingsManager::editorFont() const
{
    return _settings.value("editor/font").value<QFont>();
}

void SettingsManager::setEditorFont(const QFont& font)
{
    _settings.setValue("editor/font", font);
    emit editorSettingsChanged();
}

int SettingsManager::tabSize() const
{
    return _settings.value("editor/tabSize").toInt();
}

void SettingsManager::setTabSize(int size)
{
    _settings.setValue("editor/tabSize", size);
    emit editorSettingsChanged();
}

bool SettingsManager::wordWrap() const
{
    return _settings.value("editor/wordWrap").toBool();
}

void SettingsManager::setWordWrap(bool enabled)
{
    _settings.setValue("editor/wordWrap", enabled);
    emit editorSettingsChanged();
}

bool SettingsManager::lineNumbers() const
{
    return _settings.value("editor/lineNumbers").toBool();
}

void SettingsManager::setLineNumbers(bool enabled)
{
    _settings.setValue("editor/lineNumbers", enabled);
    emit editorSettingsChanged();
}

// Sync settings
int SettingsManager::syncInterval() const
{
    return _settings.value("sync/interval").toInt();
}

void SettingsManager::setSyncInterval(int minutes)
{
    _settings.setValue("sync/interval", minutes);
    emit syncSettingsChanged();
}

bool SettingsManager::autoSync() const
{
    return _settings.value("sync/autoSync").toBool();
}

void SettingsManager::setAutoSync(bool enabled)
{
    _settings.setValue("sync/autoSync", enabled);
    emit syncSettingsChanged();
}

// Window settings
QByteArray SettingsManager::windowGeometry() const
{
    return _settings.value("window/geometry").toByteArray();
}

void SettingsManager::setWindowGeometry(const QByteArray& geometry)
{
    _settings.setValue("window/geometry", geometry);
    emit windowSettingsChanged();
}

QByteArray SettingsManager::windowState() const
{
    return _settings.value("window/state").toByteArray();
}

void SettingsManager::setWindowState(const QByteArray& state)
{
    _settings.setValue("window/state", state);
    emit windowSettingsChanged();
}

// Authentication settings
bool SettingsManager::rememberMe() const
{
    return _settings.value("auth/rememberMe").toBool();
}

void SettingsManager::setRememberMe(bool enabled)
{
    _settings.setValue("auth/rememberMe", enabled);
    emit authSettingsChanged();
}

QString SettingsManager::lastEmail() const
{
    return _settings.value("auth/lastEmail").toString();
}

void SettingsManager::setLastEmail(const QString& email)
{
    _settings.setValue("auth/lastEmail", email);
    emit authSettingsChanged();
} 