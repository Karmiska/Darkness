#include "LogListModel.h"
#include "tools/Debug.h"
#include <QtCore/QSettings>
#include <QDebug>
#include <QFileInfo>

#include <iostream>
#include <chrono>
#include <ctime>

using namespace std::chrono;

LogListModel::LogListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    QObject::connect(this, SIGNAL(entryAdded(const QString&)), this, SLOT(addEntry(const QString&)), Qt::ConnectionType::QueuedConnection);
    customDebugMessageHandler = [this](const engine::string& msg)
    {
        system_clock::time_point p = system_clock::now();
        std::time_t t = system_clock::to_time_t(p);
        // for example : Tue Sep 27 14:21:13 2011
        engine::string timestamp = std::ctime(&t);
        timestamp = timestamp.substr(0, timestamp.length() - 1);

        engine::string filteredMsg = msg;
        engine::string look = "): ";
        auto found = msg.find(look);
        if (found != engine::string::npos)
        {
            found += look.length();
            filteredMsg = msg.substr(found, msg.length() - found);
        }
        emit this->entryAdded(QString::fromStdString(std::string(timestamp.c_str()) + ": " + std::string(filteredMsg.c_str())));
    };
}

LogListModel::~LogListModel()
{
    customDebugMessageHandler = {};
}

void LogListModel::addEntry(const QString& entry)
{
    beginResetModel();
    while (logItems.size() >= MaxVisibleLogItems)
    {
        delete logItems.last();
        logItems.erase(logItems.end() - 1);
    }
    logItems.insert(0, new LogItem(entry));
    endResetModel();
}

QHash<int, QByteArray> LogListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[EntryRole] = "entry";
    return roles;
}

int LogListModel::rowCount(const QModelIndex& /*parentP*/) const
{
    // this is a bit of a hack.
    // for some reason if the ScrollBar content list
    // has margins, it doesn't properly show the content
    if (logItems.length() > 8)
    {
        return logItems.length() + 1;
    }
    else
    {
        return logItems.length();
    }
}

QVariant LogListModel::data(const QModelIndex& indexP, int roleP) const
{
    if (roleP == EntryRole) {
        if (indexP.row() < logItems.size())
            return logItems[indexP.row()]->entry();
        else
            return "";
    }
    return QVariant();
}

void LogListModel::refreshData()
{
    beginResetModel();
    endResetModel();
}

bool LogListModel::setData(const QModelIndex &/*indexP*/, const QVariant& /*valueP*/, int /*roleP*/)
{
    beginResetModel();
    endResetModel();
    return true;
}

void LogListModel::setData(const int rowP, const QVariant& valueP, int roleP)
{
    setData(index(rowP), valueP, roleP);
}

void LogListModel::remove(const int rowP)
{
    beginResetModel();
    logItems.removeAt(rowP);
    endResetModel();
}
