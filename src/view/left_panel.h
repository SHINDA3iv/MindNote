#ifndef LEFT_PANEL_H
#define LEFT_PANEL_H

#include <QWidget>

class LeftPanel : public QWidget
{
    Q_OBJECT

public:
    LeftPanel(QWidget *parent = nullptr);
    ~LeftPanel() = default;
};

#endif // LEFT_PANEL_H
