#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QColorDialog>
#include <QFontDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onThemeChanged();
    void onEditorSettingsChanged();
    void onSyncSettingsChanged();
    void onApplyClicked();
    void onResetClicked();

private:
    void initThemeTab();
    void initEditorTab();
    void initSyncTab();
    void loadSettings();
    void saveSettings();

    QTabWidget* _tabWidget;
    QPushButton* _applyButton;
    QPushButton* _resetButton;
    QPushButton* _cancelButton;

    // Theme settings
    QComboBox* _themeCombo;
    QPushButton* _primaryColorButton;
    QPushButton* _secondaryColorButton;
    QCheckBox* _darkModeCheck;

    // Editor settings
    QPushButton* _fontButton;
    QSpinBox* _tabSizeSpin;
    QCheckBox* _wordWrapCheck;
    QCheckBox* _lineNumbersCheck;

    // Sync settings
    QSpinBox* _syncIntervalSpin;
    QCheckBox* _autoSyncCheck;
};

#endif // SETTINGS_DIALOG_H 