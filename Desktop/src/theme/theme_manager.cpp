#include "theme_manager.h"
#include "styles.h"
#include <QApplication>
#include <QPalette>

ThemeManager &ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    initConnections();
    updateColors();
}

void ThemeManager::initConnections()
{
    connect(&SettingsManager::instance(), &SettingsManager::themeChanged, this, [this]() {
        updateColors();
        applyTheme();
    });
}

void ThemeManager::updateColors()
{
    auto &settings = SettingsManager::instance();
    _primaryColor = settings.primaryColor();
    _secondaryColor = settings.secondaryColor();

    // Новая логика выбора темы
    QString theme = settings.theme();
    bool dark = false;
    if (theme == "Темная") dark = true;
    else if (theme == "Светлая") dark = false;
    else /* Системная */ dark = settings.darkMode();

    if (dark) {
        _backgroundColor = QColor("#181A20");
        _surfaceColor = QColor("#23262F");
        _textColor = QColor("#F4F4F4");
        _textSecondaryColor = QColor(244, 244, 244, 150); // 60% opacity
        _borderColor = QColor("#2A2D3A");
        _hoverColor = QColor("#31344B");
        _selectionColor = QColor("#377DFF");
        _errorColor = QColor("#FF5C5C");
        _successColor = QColor("#4CAF50");
        _warningColor = QColor("#FFC107");
    } else {
        _backgroundColor = QColor("#F4F6FB");
        _surfaceColor = QColor("#FFFFFF");
        _textColor = QColor("#23262F");
        _textSecondaryColor = QColor(35, 38, 47, 150); // 60% opacity
        _borderColor = QColor("#E0E3EB");
        _hoverColor = QColor("#E9EBF5");
        _selectionColor = QColor("#377DFF");
        _errorColor = QColor("#FF5C5C");
        _successColor = QColor("#4CAF50");
        _warningColor = QColor("#FFC107");
    }

    Styles::instance().updateColors(_primaryColor, _secondaryColor, _backgroundColor, _surfaceColor,
                                    _textColor, _textSecondaryColor, _borderColor, _hoverColor,
                                    _selectionColor);
}

QString ThemeManager::generateStylesheet() const
{
    auto &styles = Styles::instance();
    return styles.getGlobalStyles() + styles.getButtonStyles() + styles.getLineEditStyles()
           + styles.getComboBoxStyles() + styles.getCheckBoxStyles() + styles.getSpinBoxStyles()
           + styles.getScrollBarStyles() + styles.getTabStyles() + styles.getGroupBoxStyles()
           + styles.getMenuStyles() + styles.getTooltipStyles() + styles.getWorkspaceStyles()
           + styles.getSplitterStyles() + styles.getEditorStyles();
}

void ThemeManager::applyTheme()
{
    qApp->setStyleSheet(generateStylesheet());
}

QString ThemeManager::getStylesheet() const
{
    return generateStylesheet();
}

QColor ThemeManager::getColor(const QString &role) const
{
    if (role == "primary")
        return _primaryColor;
    if (role == "secondary")
        return _secondaryColor;
    if (role == "background")
        return _backgroundColor;
    if (role == "surface")
        return _surfaceColor;
    if (role == "text")
        return _textColor;
    if (role == "textSecondary")
        return _textSecondaryColor;
    if (role == "border")
        return _borderColor;
    if (role == "hover")
        return _hoverColor;
    if (role == "selection")
        return _selectionColor;
    if (role == "error")
        return _errorColor;
    if (role == "success")
        return _successColor;
    if (role == "warning")
        return _warningColor;
    return QColor();
}

bool ThemeManager::isDarkMode() const
{
    return SettingsManager::instance().darkMode();
}
