#include "InitialLayout.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

InitialLayout::InitialLayout(const QString& projectName, const QString& location)
    : m_settings{ location }
{
    m_settings.projectSettings.name(projectName);
    m_settings.projectSettings.contentPath("content");
    m_settings.projectSettings.shaderPath("shaders");
    m_settings.projectSettings.processedAssetsPath("processed");
    
    writeFolderStructure(location);
}

void makeFolderIfNotExist(const QString& path)
{
    QDir targetpath(path);
    if (!targetpath.exists())
    {
        targetpath.mkpath(".");
    }
}

void recursiveFolderCopy(const QString& src, const QString& dst)
{
    QDir srcDir(src);
    QDir dstDir(dst);
    for(auto&& folder : srcDir.entryList(
        QDir::NoDotAndDotDot | 
        QDir::System | 
        QDir::Hidden | 
        QDir::AllDirs))
    {
        QString srcFolder = QDir::toNativeSeparators(srcDir.absolutePath() + QDir::separator() + folder);
        QString dstFolder = QDir::toNativeSeparators(dstDir.absolutePath() + QDir::separator() + folder);
        makeFolderIfNotExist(dstFolder);
        recursiveFolderCopy(srcFolder, dstFolder);
    }
    for (auto&& file : srcDir.entryList(
        QDir::NoDotAndDotDot |
        QDir::System |
        QDir::Hidden |
        QDir::Files
        ))
    {
        QString srcFile = QDir::toNativeSeparators(srcDir.absolutePath() + QDir::separator() + file);
        QString dstFile = QDir::toNativeSeparators(dstDir.absolutePath() + QDir::separator() + file);
        QFile::copy(srcFile, dstFile);
    }
}

void InitialLayout::writeFolderStructure(const QString& rootLocation)
{
    QFileInfo info(rootLocation);

    QDir contentPath(info.absolutePath() + QDir::separator() + m_settings.projectSettings.contentPath());
    if (!contentPath.exists())
    {
        contentPath.mkpath(".");
    }

    QDir shaderPath(info.absolutePath() + QDir::separator() + m_settings.projectSettings.shaderPath());
    if (!shaderPath.exists())
    {
        shaderPath.mkpath(".");
    }

    recursiveFolderCopy(shaderSourceFolder(), shaderPath.absolutePath());

    QDir processedAssetsPath(info.absolutePath() + QDir::separator() + m_settings.projectSettings.processedAssetsPath());
    if (!processedAssetsPath.exists())
    {
        processedAssetsPath.mkpath(".");
    }
}

QString InitialLayout::shaderSourceFolder()
{
    return QFileInfo(QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() +
        QDir::separator() + "../../../../darkness-engine/bin/x64/Debug/shaders").absoluteFilePath();
}
