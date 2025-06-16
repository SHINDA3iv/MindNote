#include "styles.h"

Styles &Styles::instance()
{
    static Styles instance;
    return instance;
}

void Styles::updateColors(const QColor &primaryColor,
                          const QColor &secondaryColor,
                          const QColor &backgroundColor,
                          const QColor &surfaceColor,
                          const QColor &textColor,
                          const QColor &textSecondaryColor,
                          const QColor &borderColor,
                          const QColor &hoverColor,
                          const QColor &selectionColor)
{
    _primaryColor = primaryColor;
    _secondaryColor = secondaryColor;
    _backgroundColor = backgroundColor;
    _surfaceColor = surfaceColor;
    _textColor = textColor;
    _textSecondaryColor = textSecondaryColor;
    _borderColor = borderColor;
    _hoverColor = hoverColor;
    _selectionColor = selectionColor;
}

QString Styles::getGlobalStyles() const
{
    return QString(R"(
         QWidget {
             background-color: %1;
             color: %2;
             font-family: 'Segoe UI', Arial, sans-serif;
             font-size: 14px;
         }
         a, QLabel[link="true"] {
             color: %3;
             text-decoration: underline;
         }
     )")
     .arg(_backgroundColor.name())
     .arg(_textColor.name())
     .arg(_secondaryColor.name());
}

QString Styles::getButtonStyles() const
{
    return QString(R"(
        QPushButton, QToolButton {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 8px 16px;
            min-width: 80px;
        }
        QPushButton[secondary="true"] {
            background-color: %4;
            color: white;
            border: 1px solid %4;
        }
        QPushButton[secondary="true"]:hover {
            background-color: %5;
            border-color: %5;
        }
        QPushButton:hover, QToolButton:hover {
            background-color: %6;
            border-color: %7;
            color: %7;
        }
        QPushButton:pressed, QToolButton:pressed {
            background-color: %7;
            border-color: %7;
            color: %2;
        }
        QPushButton:disabled, QToolButton:disabled {
            background-color: %8;
            color: %9;
            border-color: %9;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: %6;
            border-radius: 4px;
        }
        QMenuBar::item:pressed {
            background-color: %7;
            border-radius: 4px;
        }
        QMenuBar::item:icon {
            padding-left: 8px;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_textColor.name())
     .arg(_borderColor.name())
     .arg(_secondaryColor.name())
     .arg(_secondaryColor.lighter(120).name())
     .arg(_hoverColor.name())
     .arg(_primaryColor.name())
     .arg(_backgroundColor.name())
     .arg(_textSecondaryColor.name());
}

QString Styles::getLineEditStyles() const
{
    return QString(R"(
        QLineEdit {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 4px;
            padding: 8px;
        }

        QLineEdit:focus {
            border-color: %3;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getComboBoxStyles() const
{
    return QString(R"(
        QComboBox {
            background-color: %1;
            border: 1px solid %2;
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
            width: 12px;
            height: 12px;
            background: %3;
            border-radius: 2px;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getCheckBoxStyles() const
{
    return QString(R"(
        QCheckBox {
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid %1;
            border-radius: 3px;
        }

        QCheckBox::indicator:checked {
            background-color: %2;
            border-color: %2;
        }
    )")
     .arg(_borderColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getSpinBoxStyles() const
{
    return QString(R"(
        QSpinBox {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 4px;
            padding: 8px;
        }

        QSpinBox:focus {
            border-color: %3;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getScrollBarStyles() const
{
    return QString(R"(
        QScrollBar:vertical {
            border: none;
            background: %1;
            width: 10px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background: %2;
            min-height: 20px;
            border-radius: 5px;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            border: none;
            background: %1;
            height: 10px;
            margin: 0px;
        }

        QScrollBar::handle:horizontal {
            background: %2;
            min-width: 20px;
            border-radius: 5px;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name());
}

QString Styles::getTabStyles() const
{
    return QString(R"(
        QTabWidget::pane {
            border: 1px solid %1;
            border-radius: 4px;
        }

        QTabBar::tab {
            background-color: %2;
            border: 1px solid %1;
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
            background-color: %5;
        }
    )")
     .arg(_borderColor.name())
     .arg(_surfaceColor.name())
     .arg(_primaryColor.name())
     .arg(_textColor.name())
     .arg(_hoverColor.name());
}

QString Styles::getGroupBoxStyles() const
{
    return QString(R"(
        QGroupBox {
            border: 1px solid %1;
            border-radius: 4px;
            margin-top: 1em;
            padding-top: 1em;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
        }
    )")
     .arg(_borderColor.name());
}

QString Styles::getMenuStyles() const
{
    return QString(R"(
        QMenu {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 4px;
        }

        QMenu::item {
            padding: 8px 24px 8px 24px;
            background-color: transparent;
        }

        QMenu::item:selected {
            background-color: %3;
            border-radius: 4px;
        }

        QMenu::item:pressed {
            background-color: %4;
            border-radius: 4px;
        }

        QMenu::icon {
            padding-left: 24px;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name())
     .arg(_hoverColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getTooltipStyles() const
{
    return QString(R"(
        QToolTip {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 4px;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_textColor.name())
     .arg(_borderColor.name());
}

QString Styles::getWorkspaceStyles() const
{
    return QString(R"(
        QTreeView {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 4px;
        }

        QTreeView::item {
            padding: 4px;
        }

        QTreeView::item:selected {
            background-color: %3;
            color: %4;
        }

        QTreeView::item:hover:!selected {
            background-color: %5;
        }

        QToolButton {
            background-color: %1;
            color: %4;
            border: 1px solid %2;
            padding: 4px;
            border-radius: 4px;
        }

        QToolButton:hover {
            background-color: %5;
            border-color: %6;
            color: %6;
        }

        QToolButton:pressed {
            background-color: %6;
            border-color: %6;
            color: %4;
        }

        QToolButton:disabled {
            background-color: %7;
            color: %8;
            border-color: %8;
        }
    )")
     .arg(_surfaceColor.name())
     .arg(_borderColor.name())
     .arg(_selectionColor.name())
     .arg(_textColor.name())
     .arg(_hoverColor.name())
     .arg(_primaryColor.name())
     .arg(_backgroundColor.name())
     .arg(_textSecondaryColor.name());
}

QString Styles::getSplitterStyles() const
{
    return QString(R"(
        QSplitter::handle {
            background-color: %1;
            width: 1px;
        }

        QSplitter::handle:hover {
            background-color: %2;
        }
    )")
     .arg(_borderColor.name())
     .arg(_primaryColor.name());
}

QString Styles::getEditorStyles() const
{
    return QString(R"(
        /* Область редактирования пространства */
        QWidget#EditorWidget, QWidget#Workspace, QWidget[editorArea="true"] {
            background-color: %1;
        }
        QWidget#EditorWidget QLabel, QWidget#Workspace QLabel, QWidget[editorArea="true"] QLabel {
            color: %2;
        }
        QWidget#EditorWidget QToolButton, QWidget#Workspace QToolButton, QWidget[editorArea="true"] QToolButton {
            background-color: %3;
            color: white;
            border: 1px solid %3;
        }
        QWidget#EditorWidget QToolButton:hover, QWidget#Workspace QToolButton:hover, QWidget[editorArea="true"] QToolButton:hover {
            background-color: %3;
            border-color: %3;
        }
        QWidget#EditorWidget QLineEdit, QWidget#Workspace QLineEdit, QWidget[editorArea="true"] QLineEdit {
            background-color: %5;
            border: 1px solid %3;
            color: %2;
        }
        QWidget#EditorWidget QCheckBox, QWidget#Workspace QCheckBox, QWidget[editorArea="true"] QCheckBox {
            color: %2;
        }
        QWidget#EditorWidget QCheckBox::indicator:checked, QWidget#Workspace QCheckBox::indicator:checked, QWidget[editorArea="true"] QCheckBox::indicator:checked {
            background-color: %3;
            border-color: %3;
        }
        QWidget#EditorWidget QComboBox, QWidget#Workspace QComboBox, QWidget[editorArea="true"] QComboBox {
            background-color: %5;
            border: 1px solid %3;
            color: %2;
        }
        /* QMenu для toolMenu */
        QWidget#EditorWidget QMenu::item, QWidget#Workspace QMenu::item, QWidget[editorArea="true"] QMenu::item {
            background-color: transparent;
            color: %2;
        }
        QWidget#EditorWidget QMenu::item:selected, QWidget#Workspace QMenu::item:selected, QWidget[editorArea="true"] QMenu::item:selected {
            background-color: %3;
            color: white;
        }
        QWidget#EditorWidget QMenu::item:pressed, QWidget#Workspace QMenu::item:pressed, QWidget[editorArea="true"] QMenu::item:pressed {
            background-color: %4;
            color: white;
        }
    )")
        .arg(_secondaryColor.name())
        .arg(_textColor.name())
        .arg(_primaryColor.name())
        .arg(_secondaryColor.lighter(120).name())
        .arg(_surfaceColor.name())
        .arg(_secondaryColor.lighter(105).name());
}
