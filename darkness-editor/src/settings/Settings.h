#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

#include "ProjectSettings.h"
#include "BrowserSettings.h"

class Settings
{
public:
    Settings();
    Settings(const QString& projectFilePath);
    ~Settings();

    ProjectSettings projectSettings;
    BrowserSettings browserSettings;

    QString contentPathAbsolute() const;
    QString shaderPathAbsolute() const;
    QString processedAssetsPathAbsolute() const;
    QString lastLoadedScene() const;
    void lastLoadedScene(const QString& path);
private:
    QString m_projectFilePath;
    QString projectPath() const;
    QString m_lastLoadedProjectPath;
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;
};
