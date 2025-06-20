#include "text_item.h"
#include <QToolBar>
#include <QAction>
#include <QTextCursor>
#include <QTextListFormat>

TextItem::TextItem(const QString &text, Workspace *parent) :
    ResizableItem(parent),
    _textEdit(new QTextEdit(this))
{
    _textEdit->setAcceptRichText(true);
    _textEdit->setHtml(text);

    // Создаем панель инструментов
    QToolBar *toolBar = new QToolBar(this);

    // Кнопка для жирного текста
    QAction *boldAction = toolBar->addAction("B");
    boldAction->setCheckable(true);
    boldAction->setFont(QFont("", 10, QFont::Bold));

    // Кнопка для курсивного текста
    QAction *italicAction = toolBar->addAction("I");
    italicAction->setCheckable(true);
    italicAction->setFont(QFont("", 10, QFont::StyleItalic));

    // Кнопка для маркированного списка
    QAction *unorderedListAction = toolBar->addAction("• Список");

    // Кнопка для нумерованного списка
    QAction *orderedListAction = toolBar->addAction("1. Список");

    // Подключаем действия к слотам
    connect(boldAction, &QAction::triggered, this, &TextItem::toggleBold);
    connect(italicAction, &QAction::triggered, this, &TextItem::toggleItalic);
    connect(unorderedListAction, &QAction::triggered, this, &TextItem::insertUnorderedList);
    connect(orderedListAction, &QAction::triggered, this, &TextItem::insertOrderedList);

    // Устанавливаем макет
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(toolBar);
    layout->addWidget(_textEdit);
    setLayout(layout);
    resize(width(), 250);
    
    _textEdit->installEventFilter(this);
}

QString TextItem::type() const
{
    return "TextItem";
}

QJsonObject TextItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["content"] = _textEdit->toHtml();
    return json;
}

void TextItem::deserialize(const QJsonObject &json)
{
    if (json.contains("content")) {
        _textEdit->setHtml(json["content"].toString());
    }
}

void TextItem::toggleBold()
{
    QTextCharFormat format;
    format.setFontWeight(_textEdit->fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
    mergeCurrentCharFormat(format);
}

void TextItem::toggleItalic()
{
    QTextCharFormat format;
    format.setFontItalic(!_textEdit->fontItalic());
    mergeCurrentCharFormat(format);
}

void TextItem::insertUnorderedList()
{
    QTextCursor cursor = _textEdit->textCursor();
    QTextListFormat listFormat;
    listFormat.setStyle(QTextListFormat::ListDisc); // Маркированный список
    cursor.createList(listFormat);
}

void TextItem::insertOrderedList()
{
    QTextCursor cursor = _textEdit->textCursor();
    QTextListFormat listFormat;
    listFormat.setStyle(QTextListFormat::ListDecimal); // Нумерованный список
    cursor.createList(listFormat);
}

void TextItem::mergeCurrentCharFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = _textEdit->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    QTextCharFormat currentFormat = cursor.charFormat();
    currentFormat.merge(format);
    cursor.setCharFormat(currentFormat);
    _textEdit->mergeCurrentCharFormat(format);
}
