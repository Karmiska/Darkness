#include "Settings.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

Settings::Settings()
    : m_projectFilePath{ "" }
{
}

Settings::Settings(const QString& projectFilePath)
    : m_projectFilePath{ projectFilePath }
{
    QFile projectFile(projectFilePath);
    if (projectFile.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc(QJsonDocument::fromJson(projectFile.readAll()));
        read(doc.object());
        projectFile.close();
    }
}

Settings::~Settings()
{
    if (m_projectFilePath != "")
    {
        QFile projectFile(m_projectFilePath);
        if (projectFile.open(QIODevice::WriteOnly))
        {
            QJsonObject settings;
            write(settings);
            QJsonDocument doc(settings);
            projectFile.write(doc.toJson());
            projectFile.close();
        }
    }
}

void Settings::read(const QJsonObject& json)
{
    projectSettings.read(json["projectSettings"].toObject());
    browserSettings.read(json["browserSettings"].toObject());
    m_lastLoadedProjectPath = json["lastLoadedScene"].toString();
}

void Settings::write(QJsonObject& json) const
{
    QJsonObject projectSettingsObj;
    projectSettings.write(projectSettingsObj);

    QJsonObject browserSettingsObj;
    browserSettings.write(browserSettingsObj);

    json["projectSettings"] = projectSettingsObj;
    json["browserSettings"] = browserSettingsObj;
    json["lastLoadedScene"] = m_lastLoadedProjectPath;
}

QString Settings::projectPath() const
{
    return QFileInfo(m_projectFilePath).absolutePath();
}

QString Settings::contentPathAbsolute() const
{
    return QDir::toNativeSeparators(projectPath() + QDir::separator() + projectSettings.contentPath() + QDir::separator());
}

QString Settings::processedAssetsPathAbsolute() const
{
    return QDir::toNativeSeparators(projectPath() + QDir::separator() + projectSettings.processedAssetsPath() + QDir::separator());
}

QString Settings::shaderPathAbsolute() const
{
    return QDir::toNativeSeparators(projectPath() + QDir::separator() + projectSettings.shaderPath() + QDir::separator());
}

QString Settings::lastLoadedScene() const
{
    return m_lastLoadedProjectPath;
}

void Settings::lastLoadedScene(const QString& path)
{
    m_lastLoadedProjectPath = path;
}

