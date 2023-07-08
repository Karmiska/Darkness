#pragma once

#include <QtCore/QObject>

class LogItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString entry READ entry WRITE setEntry NOTIFY entryChanged)
public:
    LogItem(QObject *parent = 0);
    LogItem(QString entry, QObject *parent = 0);
    QString entry() const;
    void setEntry(const QString& entry);

signals:
    void entryChanged();

private:
    QString m_entry;
};
