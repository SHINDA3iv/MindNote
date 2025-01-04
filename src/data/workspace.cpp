#include "workspace.h"

WorkSpace::WorkSpace(const QString &name,
                     const QString &link,
                     const QIcon &icon,
                     const QDate &startDate,
                     const QDate &endDate) :
    _name(name),
    _link(link),
    _icon(icon),
    _startDate(startDate),
    _endDate(endDate)
{}

QString WorkSpace::name() const
{
    return _name;
}

QString WorkSpace::link() const
{
    return _link;
}

QIcon WorkSpace::icon() const
{
    return _icon;
}

QDate WorkSpace::startDate() const
{
    return _startDate;
}

QDate WorkSpace::endDate() const
{
    return _endDate;
}

void WorkSpace::setName(const QString &name)
{
    _name = name;
}

void WorkSpace::setLink(const QString &link)
{
    _link = link;
}
void WorkSpace::setIcon(const QIcon &icon)
{
    _icon = icon;
}

void WorkSpace::setStartDate(const QDate &startDate)
{
    _startDate = startDate;
}

void WorkSpace::setEndDate(const QDate &endDate)
{
    _endDate = endDate;
}
