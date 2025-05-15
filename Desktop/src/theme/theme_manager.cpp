#include "theme_manager.h"
#include "styles.h"
#include <QApplication>
#include <QPalette>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
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
    auto& settings = SettingsManager::instance();
    _primaryColor = settings.primaryColor();
    _secondaryColor = settings.secondaryColor();

    if (settings.darkMode()) {
        _backgroundColor = QColor("#121212");
        _surfaceColor = QColor("#1E1E1E");
        _textColor = QColor("#FFFFFF");
        _textSecondaryColor = QColor("#B3FFFFFF");
        _borderColor = QColor("#2C2C2C");
        _hoverColor = QColor("#2C2C2C");
        _selectionColor = QColor("#2196F3");
        _errorColor = QColor("#CF6679");
        _successColor = QColor("#4CAF50");
        _warningColor = QColor("#FFC107");
    } else {
        _backgroundColor = QColor("#FFFFFF");
        _surfaceColor = QColor("#F5F5F5");
        _textColor = QColor("#000000");
        _textSecondaryColor = QColor("#99000000");
        _borderColor = QColor("#E0E0E0");
        _hoverColor = QColor("#F5F5F5");
        _selectionColor = QColor("#2196F3");
        _errorColor = QColor("#B00020");
        _successColor = QColor("#4CAF50");
        _warningColor = QColor("#FFC107");
    }

    // Update styles with new colors
    Styles::instance().updateColors(
        _primaryColor,
        _secondaryColor,
        _backgroundColor,
        _surfaceColor,
        _textColor,
        _textSecondaryColor,
        _borderColor,
        _hoverColor,
        _selectionColor
    );
}

QString ThemeManager::generateStylesheet() const
{
    auto& styles = Styles::instance();
    return styles.getGlobalStyles() +
           styles.getButtonStyles() +
           styles.getLineEditStyles() +
           styles.getComboBoxStyles() +
           styles.getCheckBoxStyles() +
           styles.getSpinBoxStyles() +
           styles.getScrollBarStyles() +
           styles.getTabStyles() +
           styles.getGroupBoxStyles() +
           styles.getMenuStyles() +
           styles.getTooltipStyles() +
           styles.getWorkspaceStyles() +
           styles.getSplitterStyles();
}

void ThemeManager::applyTheme()
{
    qApp->setStyleSheet(generateStylesheet());
}

QString ThemeManager::getStylesheet() const
{
    return generateStylesheet();
}

QColor ThemeManager::getColor(const QString& role) const
{
    if (role == "primary") return _primaryColor;
    if (role == "secondary") return _secondaryColor;
    if (role == "background") return _backgroundColor;
    if (role == "surface") return _surfaceColor;
    if (role == "text") return _textColor;
    if (role == "textSecondary") return _textSecondaryColor;
    if (role == "border") return _borderColor;
    if (role == "hover") return _hoverColor;
    if (role == "selection") return _selectionColor;
    if (role == "error") return _errorColor;
    if (role == "success") return _successColor;
    if (role == "warning") return _warningColor;
    return QColor();
}

bool ThemeManager::isDarkMode() const
{
    return SettingsManager::instance().darkMode();
} 
