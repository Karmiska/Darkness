#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include "settings/Settings.h"

class InitialLayout
{
public:
    InitialLayout(const QString& projectName, const QString& location);

private:
    Settings m_settings;
    void writeFolderStructure(const QString& location);
    QString shaderSourceFolder();
};
