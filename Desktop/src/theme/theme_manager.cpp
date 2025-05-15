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
        _backgroundColor = QColor(0x12, 0x12, 0x12); // #121212
        _surfaceColor = QColor(0x1E, 0x1E, 0x1E); // #1E1E1E
        _textColor = QColor(0xFF, 0xFF, 0xFF); // #FFFFFF
        _textSecondaryColor = QColor(0xB3, 0xFF, 0xFF, 0xFF); // #B3FFFFFF
        _borderColor = QColor(0x2C, 0x2C, 0x2C); // #2C2C2C
        _hoverColor = QColor(0x2C, 0x2C, 0x2C); // #2C2C2C
        _selectionColor = QColor(0x21, 0x96, 0xF3); // #2196F3
        _errorColor = QColor(0xCF, 0x66, 0x79); // #CF6679
        _successColor = QColor(0x4C, 0xAF, 0x50); // #4CAF50
        _warningColor = QColor(0xFF, 0xC1, 0x07); // #FFC107
    } else {
        _backgroundColor = QColor(0xFF, 0xFF, 0xFF); // #FFFFFF
        _surfaceColor = QColor(0xF5, 0xF5, 0xF5); // #F5F5F5
        _textColor = QColor(0x00, 0x00, 0x00); // #000000
        _textSecondaryColor = QColor(0x99, 0x00, 0x00, 0x00); // #99000000
        _borderColor = QColor(0xE0, 0xE0, 0xE0); // #E0E0E0
        _hoverColor = QColor(0x05, 0xF5, 0xF5); // #F5F5F5
        _selectionColor = QColor(0x21, 0x96, 0xF3); // #2196F3
        _errorColor = QColor(0xB0, 0x00, 0x20); // #B00020
        _successColor = QColor(0x4C, 0xAF, 0x50); // #4CAF50
        _warningColor = QColor(0xFF, 0xC1, 0x07); // #FFC107
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
