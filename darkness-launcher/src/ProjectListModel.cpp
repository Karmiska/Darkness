#include "projectlistmodel.h"
#include <QtCore/QSettings>
#include <QDebug>
#include <QFileInfo>
#include "InitialLayout.h"
#include "EditorStartup.h"

ProjectListModel::ProjectListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    loadRecentProjects();
}

ProjectListModel::~ProjectListModel()
{
    saveRecentProjects();
}

void ProjectListModel::createProject(const QString& name, const QString& location)
{
    beginResetModel();
    projectItems.insert(0, new ProjectItem(name, location));
    endResetModel();

    {
        InitialLayout layout(name, location);
    }

    openProject(location);
}

void ProjectListModel::openProject(const QString& location)
{
    EditorStartup start((QObject*)this, location);
}

void ProjectListModel::loadRecentProjects()
{
    QSettings settings;
    settings.beginGroup("launcher");
    int size = settings.beginReadArray("recentProjects");
    for(int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString projectName = settings.value("projectName").toString();
        QString projectPath = settings.value("projectPath").toString();
        QFileInfo info(projectPath);
        if(info.exists())
            projectItems.append(new ProjectItem(projectName, projectPath));
    }
    settings.endArray();
    settings.endGroup();
}

void ProjectListModel::saveRecentProjects()
{
    QSettings settings;
    settings.beginGroup("launcher");
    settings.remove("recentProjects");

    settings.beginWriteArray("recentProjects", projectItems.size());
    for(int i = 0; i < projectItems.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("projectName", projectItems[i]->projectName());
        settings.setValue("projectPath", projectItems[i]->projectPath());
    }
    settings.endArray();
    settings.endGroup();
}

QHash<int, QByteArray> ProjectListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "projectName";
    roles[PathRole] = "projectPath";
    roles[LineRole] = "lineRole";
    roles[LoadRole] = "load";
    return roles;
}

int ProjectListModel::rowCount(const QModelIndex& parentP) const
{
    // this is a bit of a hack.
    // for some reason if the ScrollBar content list
    // has margins, it doesn't properly show the content
    if (projectItems.length() > 8)
    {
        return projectItems.length() + 1;
    }
    else
    {
        return projectItems.length();
    }
}

QVariant ProjectListModel::data(const QModelIndex& indexP, int roleP) const
{
    if(roleP == NameRole) {
        if (indexP.row() < projectItems.size())
            return projectItems[indexP.row()]->projectName();
        else
            return "";
    }
    else if(roleP == PathRole)
    {
        if (indexP.row() < projectItems.size())
            return projectItems[indexP.row()]->projectPath();
        else
            return "";
    }
    else if (roleP == LineRole)
    {
        if (indexP.row() < projectItems.size() - 1)
            return true;
        else
            return false;
    }
    return QVariant();
}

void ProjectListModel::refreshData()
{
    beginResetModel();
    endResetModel();
}

bool ProjectListModel::setData ( const QModelIndex &indexP, const QVariant& valueP, int roleP )
{
    beginResetModel();
    if (roleP == LoadRole) {
        openProject(projectItems[indexP.row()]->projectPath());
        return true;
    }
    endResetModel();
    return false;
}

void ProjectListModel::setData(const int rowP, const QVariant& valueP, int roleP)
{
    setData(index(rowP), valueP, roleP);
}

void ProjectListModel::remove(const int rowP)
{
    beginResetModel();
    projectItems.removeAt(rowP);
    endResetModel();
}
