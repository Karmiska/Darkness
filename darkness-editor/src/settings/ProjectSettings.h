#pragma once

#include <QString>
#include <QJsonObject>

class ProjectSettings
{
public:
    ProjectSettings();
    ProjectSettings(
        const QString& name,
        const QString& contentPath, 
        const QString& shaderPath,
        const QString& processedAssetsPath);

    QString settingsCategory() const;

    QString name() const;
    QString contentPath() const;
    QString shaderPath() const;
    QString processedAssetsPath() const;

    void name(const QString& name);
    void contentPath(const QString& contentPath);
    void shaderPath(const QString& shaderPath);
    void processedAssetsPath(const QString& processedAssetsPath);

    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

private:
    QString m_name;
    QString m_contentPath;
    QString m_shaderPath;
    QString m_processedAssetsPath;
};
