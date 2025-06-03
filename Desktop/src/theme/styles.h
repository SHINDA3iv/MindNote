#pragma once

#include <QString>
#include <QColor>

class Styles
{
public:
    static Styles &instance();

    QString getGlobalStyles() const;
    QString getButtonStyles() const;
    QString getLineEditStyles() const;
    QString getComboBoxStyles() const;
    QString getCheckBoxStyles() const;
    QString getSpinBoxStyles() const;
    QString getScrollBarStyles() const;
    QString getTabStyles() const;
    QString getGroupBoxStyles() const;
    QString getMenuStyles() const;
    QString getTooltipStyles() const;
    QString getWorkspaceStyles() const;
    QString getSplitterStyles() const;
    QString getEditorStyles() const;

    void updateColors(const QColor &primaryColor,
                      const QColor &secondaryColor,
                      const QColor &backgroundColor,
                      const QColor &surfaceColor,
                      const QColor &textColor,
                      const QColor &textSecondaryColor,
                      const QColor &borderColor,
                      const QColor &hoverColor,
                      const QColor &selectionColor);

private:
    Styles() = default;
    ~Styles() = default;
    Styles(const Styles &) = delete;
    Styles &operator=(const Styles &) = delete;

    QColor _primaryColor;
    QColor _secondaryColor;
    QColor _backgroundColor;
    QColor _surfaceColor;
    QColor _textColor;
    QColor _textSecondaryColor;
    QColor _borderColor;
    QColor _hoverColor;
    QColor _selectionColor;
};
