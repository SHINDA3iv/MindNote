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

    if (settings.darkMode()) {
        _backgroundColor = QColor(0x1A, 0x1A, 0x1A); // #1A1A1A - Slightly lighter dark background
        _surfaceColor = QColor(0x2D, 0x2D, 0x2D); // #2D2D2D - Better contrast with background
        _textColor = QColor(0xFF, 0xFF, 0xFF); // #FFFFFF
        _textSecondaryColor = QColor(0xB3, 0xFF, 0xFF, 0xFF); // #B3FFFFFF - 70% opacity
        _borderColor = QColor(0x3D, 0x3D, 0x3D); // #3D3D3D - Better visible borders
        _hoverColor = QColor(0x4D, 0x4D, 0x4D); // #4D4D4D - Darker hover for better feedback
        _selectionColor = QColor(0x1E, 0x88, 0xE5); // #1E88E5 - Lighter blue for better visibility
        _errorColor = QColor(0xEF, 0x53, 0x50); // #EF5350 - Softer red
        _successColor = QColor(0x66, 0xBB, 0x6A); // #66BB6A - Softer green
        _warningColor = QColor(0xFF, 0xA7, 0x26); // #FFA726 - Softer amber
    } else {
        _backgroundColor = QColor(0xFF, 0xFF, 0xFF); // #FFFFFF
        _surfaceColor = QColor(0xF8, 0xF9, 0xFA); // #F8F9FA - Slightly off-white
        _textColor = QColor(0x21, 0x21, 0x21); // #212121 - Darker text for better readability
        _textSecondaryColor = QColor(0x75, 0x75, 0x75); // #757575 - Medium gray
        _borderColor = QColor(0xE0, 0xE0, 0xE0); // #E0E0E0
        _hoverColor = QColor(0xE0, 0xE0, 0xE0); // #E0E0E0 - Darker hover for better feedback
        _selectionColor = QColor(0x1E, 0x88, 0xE5); // #1E88E5 - Matching primary blue
        _errorColor = QColor(0xD3, 0x2F, 0x2F); // #D32F2F - Deeper red
        _successColor = QColor(0x2E, 0x7D, 0x32); // #2E7D32 - Deeper green
        _warningColor = QColor(0xF5, 0x7C, 0x00); // #F57C00 - Deeper amber
    }

    // Update styles with new colors
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
           + styles.getSplitterStyles();
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
