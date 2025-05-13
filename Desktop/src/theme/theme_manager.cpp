#include "theme_manager.h"
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
}

QString ThemeManager::generateStylesheet() const
{
    QString style = QString(R"(
        /* Global styles */
        QWidget {
            background-color: %1;
            color: %2;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 14px;
        }

        /* Buttons */
        QPushButton, QToolButton {
            background-color: %3;
            color: %4;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 8px 16px;
            min-width: 80px;
        }

        QPushButton:hover, QToolButton:hover {
            background-color: %6;
        }

        QPushButton:pressed, QToolButton:pressed {
            background-color: %7;
        }

        QPushButton:disabled, QToolButton:disabled {
            background-color: %8;
            color: %9;
        }

        /* Line edits */
        QLineEdit {
            background-color: %10;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 8px;
        }

        QLineEdit:focus {
            border-color: %3;
        }

        /* Combo boxes */
        QComboBox {
            background-color: %10;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 8px;
            min-width: 6em;
        }

        QComboBox:hover {
            border-color: %3;
        }

        QComboBox::drop-down {
            border: none;
            width: 20px;
        }

        QComboBox::down-arrow {
            image: url(:/icons/down-arrow.png);
            width: 12px;
            height: 12px;
        }

        /* Checkboxes */
        QCheckBox {
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid %5;
            border-radius: 3px;
        }

        QCheckBox::indicator:checked {
            background-color: %3;
            border-color: %3;
        }

        /* Spin boxes */
        QSpinBox {
            background-color: %10;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 8px;
        }

        QSpinBox:focus {
            border-color: %3;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            border: none;
            background: %10;
            width: 10px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background: %11;
            min-height: 20px;
            border-radius: 5px;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            border: none;
            background: %10;
            height: 10px;
            margin: 0px;
        }

        QScrollBar::handle:horizontal {
            background: %11;
            min-width: 20px;
            border-radius: 5px;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        /* Tabs */
        QTabWidget::pane {
            border: 1px solid %5;
            border-radius: 4px;
        }

        QTabBar::tab {
            background-color: %10;
            border: 1px solid %5;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            padding: 8px 16px;
            margin-right: 2px;
        }

        QTabBar::tab:selected {
            background-color: %3;
            color: %4;
        }

        QTabBar::tab:hover:!selected {
            background-color: %6;
        }

        /* Group boxes */
        QGroupBox {
            border: 1px solid %5;
            border-radius: 4px;
            margin-top: 1em;
            padding-top: 1em;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
        }

        /* Menus */
        QMenu {
            background-color: %10;
            border: 1px solid %5;
            border-radius: 4px;
        }

        QMenu::item {
            padding: 8px 24px;
        }

        QMenu::item:selected {
            background-color: %6;
        }

        /* Tooltips */
        QToolTip {
            background-color: %10;
            color: %2;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 4px;
        }
    )")
        .arg(_backgroundColor.name())
        .arg(_textColor.name())
        .arg(_primaryColor.name())
        .arg(_textColor.name())
        .arg(_borderColor.name())
        .arg(_hoverColor.name())
        .arg(_primaryColor.darker(120).name())
        .arg(_surfaceColor.name())
        .arg(_textSecondaryColor.name())
        .arg(_surfaceColor.name())
        .arg(_textSecondaryColor.name());

    return style;
}

void ThemeManager::applyTheme()
{
    QApplication::setStyle("Fusion");
    // qApp->setStyleSheet(generateStylesheet());

    QPalette palette;
    palette.setColor(QPalette::Window, _backgroundColor);
    palette.setColor(QPalette::WindowText, _textColor);
    palette.setColor(QPalette::Base, _surfaceColor);
    palette.setColor(QPalette::AlternateBase, _backgroundColor);
    palette.setColor(QPalette::ToolTipBase, _surfaceColor);
    palette.setColor(QPalette::ToolTipText, _textColor);
    palette.setColor(QPalette::Text, _textColor);
    palette.setColor(QPalette::Button, _surfaceColor);
    palette.setColor(QPalette::ButtonText, _textColor);
    palette.setColor(QPalette::Link, _primaryColor);
    palette.setColor(QPalette::Highlight, _selectionColor);
    palette.setColor(QPalette::HighlightedText, _textColor);

    qApp->setPalette(palette);
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
