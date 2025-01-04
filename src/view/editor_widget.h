#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <QWidget>

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget() = default;
};

#endif // EDITOR_WIDGET_H
