#pragma once

#include <QAbstractListModel>
#include "LogItem.h"

constexpr int MaxVisibleLogItems = 50;

class LogListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LogListRoles
    {
        EntryRole = Qt::UserRole + 1
    };

    Q_ENUMS(LogRoles)

    LogListModel(QObject* parent = 0);
    virtual ~LogListModel();

    int rowCount(const QModelIndex& parentP = QModelIndex()) const;
    QVariant data(const QModelIndex& indexP, int roleP = Qt::DisplayRole) const;
    bool setData(const QModelIndex &indexP, const QVariant& valueP, int roleP = Qt::EditRole);
    void refreshData();

protected:
    QHash<int, QByteArray> roleNames() const;

signals:
    void entryAdded(const QString& entry);

public slots:
    void setData(const int rowP, const QVariant& valueP, int roleP);
    void remove(const int rowP);
    void addEntry(const QString& entry);

private:
    QList<LogItem*> logItems;
};
Q_DECLARE_METATYPE(LogListModel*)
