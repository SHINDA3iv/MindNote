#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include "../settings/settings_manager.h"

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    static ThemeManager& instance();

    void applyTheme();
    QString getStylesheet() const;
    QColor getColor(const QString& role) const;
    bool isDarkMode() const;

private:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void initConnections();
    void updateColors();
    QString generateStylesheet() const;

    QColor _primaryColor;
    QColor _secondaryColor;
    QColor _backgroundColor;
    QColor _surfaceColor;
    QColor _textColor;
    QColor _textSecondaryColor;
    QColor _borderColor;
    QColor _hoverColor;
    QColor _selectionColor;
    QColor _errorColor;
    QColor _successColor;
    QColor _warningColor;
};

#endif // THEME_MANAGER_H 