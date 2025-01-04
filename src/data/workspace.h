#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QDate>
#include <QIcon>
#include <QString>

class WorkSpace
{
public:
    WorkSpace(const QString &name,
              const QString &link,
              const QIcon &icon = QIcon(),
              const QDate &startDate = QDate(),
              const QDate &endDate = QDate());

    QString name() const;
    QString link() const;
    QIcon icon() const;
    QDate startDate() const;
    QDate endDate() const;

    void setName(const QString &name);
    void setLink(const QString &link);
    void setIcon(const QIcon &icon);
    void setStartDate(const QDate &startDate);
    void setEndDate(const QDate &endDate);

private:
    QString _name;
    QString _link;
    QIcon _icon;
    QDate _startDate;
    QDate _endDate;
};

#endif // WORKSPACE_H
