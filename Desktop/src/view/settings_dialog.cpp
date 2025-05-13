#include "settings_dialog.h"
#include "../settings/settings_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Настройки");
    setMinimumSize(600, 400);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Tab widget
    _tabWidget = new QTabWidget(this);
    initThemeTab();
    initEditorTab();
    initSyncTab();
    initAuthTab();
    mainLayout->addWidget(_tabWidget);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    _applyButton = new QPushButton("Применить", this);
    _resetButton = new QPushButton("Сбросить", this);
    _cancelButton = new QPushButton("Отмена", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(_applyButton);
    buttonLayout->addWidget(_resetButton);
    buttonLayout->addWidget(_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    connect(_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    loadSettings();
}

void SettingsDialog::initThemeTab()
{
    QWidget *themeTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(themeTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    // Theme selection
    QGroupBox *themeGroup = new QGroupBox("Тема оформления", themeTab);
    QFormLayout *themeLayout = new QFormLayout(themeGroup);

    _themeCombo = new QComboBox(themeGroup);
    _themeCombo->addItems({ "Светлая", "Темная", "Системная" });
    themeLayout->addRow("Тема:", _themeCombo);

    _darkModeCheck = new QCheckBox("Темный режим", themeGroup);
    themeLayout->addRow("", _darkModeCheck);

    // Colors
    QGroupBox *colorsGroup = new QGroupBox("Цвета", themeTab);
    QFormLayout *colorsLayout = new QFormLayout(colorsGroup);

    _primaryColorButton = new QPushButton(colorsGroup);
    _primaryColorButton->setFixedSize(100, 25);
    colorsLayout->addRow("Основной цвет:", _primaryColorButton);

    _secondaryColorButton = new QPushButton(colorsGroup);
    _secondaryColorButton->setFixedSize(100, 25);
    colorsLayout->addRow("Дополнительный цвет:", _secondaryColorButton);

    layout->addWidget(themeGroup);
    layout->addWidget(colorsGroup);
    layout->addStretch();

    _tabWidget->addTab(themeTab, "Внешний вид");

    // Connections
    connect(_primaryColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(SettingsManager::instance().primaryColor(), this);
        if (color.isValid()) {
            _primaryColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
        }
    });

    connect(_secondaryColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(SettingsManager::instance().secondaryColor(), this);
        if (color.isValid()) {
            _secondaryColorButton->setStyleSheet(QString("background-color: %1").arg(color.name()));
        }
    });
}

void SettingsDialog::initEditorTab()
{
    QWidget *editorTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editorTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    QGroupBox *editorGroup = new QGroupBox("Настройки редактора", editorTab);
    QFormLayout *editorLayout = new QFormLayout(editorGroup);

    _fontButton = new QPushButton(editorGroup);
    _fontButton->setText("Выбрать шрифт...");
    editorLayout->addRow("Шрифт:", _fontButton);

    _tabSizeSpin = new QSpinBox(editorGroup);
    _tabSizeSpin->setRange(2, 8);
    editorLayout->addRow("Размер табуляции:", _tabSizeSpin);

    _wordWrapCheck = new QCheckBox("Перенос строк", editorGroup);
    editorLayout->addRow("", _wordWrapCheck);

    _lineNumbersCheck = new QCheckBox("Номера строк", editorGroup);
    editorLayout->addRow("", _lineNumbersCheck);

    layout->addWidget(editorGroup);
    layout->addStretch();

    _tabWidget->addTab(editorTab, "Редактор");

    // Connections
    connect(_fontButton, &QPushButton::clicked, this, [this]() {
        bool ok;
        QFont font = QFontDialog::getFont(&ok, SettingsManager::instance().editorFont(), this);
        if (ok) {
            _fontButton->setFont(font);
            _fontButton->setText(font.family() + " " + QString::number(font.pointSize()));
        }
    });
}

void SettingsDialog::initSyncTab()
{
    QWidget *syncTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(syncTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    QGroupBox *syncGroup = new QGroupBox("Настройки синхронизации", syncTab);
    QFormLayout *syncLayout = new QFormLayout(syncGroup);

    _syncIntervalSpin = new QSpinBox(syncGroup);
    _syncIntervalSpin->setRange(1, 60);
    _syncIntervalSpin->setSuffix(" мин");
    syncLayout->addRow("Интервал синхронизации:", _syncIntervalSpin);

    _autoSyncCheck = new QCheckBox("Автоматическая синхронизация", syncGroup);
    syncLayout->addRow("", _autoSyncCheck);

    layout->addWidget(syncGroup);
    layout->addStretch();

    _tabWidget->addTab(syncTab, "Синхронизация");
}

void SettingsDialog::initAuthTab()
{
    QWidget *authTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(authTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);

    QGroupBox *authGroup = new QGroupBox("Настройки авторизации", authTab);
    QFormLayout *authLayout = new QFormLayout(authGroup);

    _rememberMeCheck = new QCheckBox("Запомнить меня", authGroup);
    authLayout->addRow("", _rememberMeCheck);

    _lastEmailEdit = new QLineEdit(authGroup);
    _lastEmailEdit->setPlaceholderText("Последний использованный email");
    authLayout->addRow("Последний email:", _lastEmailEdit);

    layout->addWidget(authGroup);
    layout->addStretch();

    _tabWidget->addTab(authTab, "Авторизация");
}

void SettingsDialog::loadSettings()
{
    auto &settings = SettingsManager::instance();

    // Theme settings
    _themeCombo->setCurrentText(settings.theme());
    _darkModeCheck->setChecked(settings.darkMode());
    _primaryColorButton->setStyleSheet(
     QString("background-color: %1").arg(settings.primaryColor().name()));
    _secondaryColorButton->setStyleSheet(
     QString("background-color: %1").arg(settings.secondaryColor().name()));

    // Editor settings
    QFont editorFont = settings.editorFont();
    _fontButton->setFont(editorFont);
    _fontButton->setText(editorFont.family() + " " + QString::number(editorFont.pointSize()));
    _tabSizeSpin->setValue(settings.tabSize());
    _wordWrapCheck->setChecked(settings.wordWrap());
    _lineNumbersCheck->setChecked(settings.lineNumbers());

    // Sync settings
    _syncIntervalSpin->setValue(settings.syncInterval());
    _autoSyncCheck->setChecked(settings.autoSync());

    // Auth settings
    _rememberMeCheck->setChecked(settings.rememberMe());
    _lastEmailEdit->setText(settings.lastEmail());
}

void SettingsDialog::saveSettings()
{
    auto &settings = SettingsManager::instance();

    // Theme settings
    settings.setTheme(_themeCombo->currentText());
    settings.setDarkMode(_darkModeCheck->isChecked());
    settings.setPrimaryColor(QColor(_primaryColorButton->styleSheet().split(":").last().trimmed()));
    settings.setSecondaryColor(
     QColor(_secondaryColorButton->styleSheet().split(":").last().trimmed()));

    // Editor settings
    settings.setEditorFont(_fontButton->font());
    settings.setTabSize(_tabSizeSpin->value());
    settings.setWordWrap(_wordWrapCheck->isChecked());
    settings.setLineNumbers(_lineNumbersCheck->isChecked());

    // Sync settings
    settings.setSyncInterval(_syncIntervalSpin->value());
    settings.setAutoSync(_autoSyncCheck->isChecked());

    // Auth settings
    settings.setRememberMe(_rememberMeCheck->isChecked());
    settings.setLastEmail(_lastEmailEdit->text());
}

void SettingsDialog::onApplyClicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::onResetClicked()
{
    if (QMessageBox::question(
         this, "Сброс настроек",
         "Вы уверены, что хотите сбросить все настройки к значениям по умолчанию?",
         QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        SettingsManager::instance().loadDefaults();
        loadSettings();
    }
}

void SettingsDialog::onThemeChanged()
{
    // Update theme preview
    _primaryColorButton->setStyleSheet(
     QString("background-color: %1").arg(SettingsManager::instance().primaryColor().name()));
    _secondaryColorButton->setStyleSheet(
     QString("background-color: %1").arg(SettingsManager::instance().secondaryColor().name()));
}

void SettingsDialog::onEditorSettingsChanged()
{
    // Update editor settings preview
    QFont editorFont = SettingsManager::instance().editorFont();
    _fontButton->setFont(editorFont);
    _fontButton->setText(editorFont.family() + " " + QString::number(editorFont.pointSize()));
}

void SettingsDialog::onSyncSettingsChanged()
{
    // Update sync settings preview
    _syncIntervalSpin->setValue(SettingsManager::instance().syncInterval());
    _autoSyncCheck->setChecked(SettingsManager::instance().autoSync());
}

void SettingsDialog::onAuthSettingsChanged()
{
    // Update auth settings preview
    _rememberMeCheck->setChecked(SettingsManager::instance().rememberMe());
    _lastEmailEdit->setText(SettingsManager::instance().lastEmail());
}
