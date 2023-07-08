#include "projectitem.h"

ProjectItem::ProjectItem(QObject *parent)
    : QObject(parent)
{

}

ProjectItem::ProjectItem(QString projectName, QString projectPath, QObject *parent)
    : QObject(parent)
    , m_projectName{ projectName }
    , m_projectPath{ projectPath }
{
}

QString ProjectItem::projectName() const
{
    return m_projectName;
}

QString ProjectItem::projectPath() const
{
    return m_projectPath;
}

void ProjectItem::setProjectName(const QString& name)
{
    if(m_projectName != name)
    {
        m_projectName = name;
        emit projectNameChanged();
    }
}

void ProjectItem::setProjectPath(const QString& path)
{
    if(m_projectPath != path)
    {
        m_projectPath = path;
        emit projectPathChanged();
    }
}
