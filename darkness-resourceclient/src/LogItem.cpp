#include "LogItem.h"

LogItem::LogItem(QObject *parent)
    : QObject(parent)
{

}

LogItem::LogItem(QString entry, QObject *parent)
    : QObject(parent)
    , m_entry{ entry }
{
}

QString LogItem::entry() const
{
    return m_entry;
}

void LogItem::setEntry(const QString& entry)
{
    if (m_entry != entry)
    {
        m_entry = entry;
        emit entryChanged();
    }
}
