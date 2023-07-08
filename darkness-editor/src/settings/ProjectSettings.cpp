#include "ProjectSettings.h"
#include <QDebug>

ProjectSettings::ProjectSettings()
{
}

ProjectSettings::ProjectSettings(
    const QString& name,
    const QString& contentPath,
    const QString& shaderPath,
    const QString& processedAssetsPath)
    : m_name{ name }
    , m_contentPath{ contentPath }
    , m_shaderPath{ shaderPath }
    , m_processedAssetsPath{ processedAssetsPath }
{
}

QString ProjectSettings::settingsCategory() const
{
    return "projectSettings";
}

QString ProjectSettings::name() const
{
    return m_name;
}

QString ProjectSettings::contentPath() const
{
    return m_contentPath;
}

QString ProjectSettings::shaderPath() const
{
    return m_shaderPath;
}

QString ProjectSettings::processedAssetsPath() const
{
    return m_processedAssetsPath;
}

void ProjectSettings::name(const QString& name)
{
    m_name = name;
}

void ProjectSettings::contentPath(const QString& contentPath)
{
    m_contentPath = contentPath;
}

void ProjectSettings::shaderPath(const QString& shaderPath)
{
    m_shaderPath = shaderPath;
}

void ProjectSettings::processedAssetsPath(const QString& processedAssetsPath)
{
    m_processedAssetsPath = processedAssetsPath;
}

void ProjectSettings::read(const QJsonObject& json)
{
    m_name = json["name"].toString();
    m_contentPath = json["contentPath"].toString();
    m_shaderPath = json["shaderPath"].toString();
    m_processedAssetsPath = json["processedAssetsPath"].toString();
}

void ProjectSettings::write(QJsonObject& json) const
{
    json["name"] = m_name;
    json["contentPath"] = m_contentPath;
    json["shaderPath"] = m_shaderPath;
    json["processedAssetsPath"] = m_processedAssetsPath;
}

