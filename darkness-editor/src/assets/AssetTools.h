#pragma once

#include <QString>
#include <QFileInfo>
#include <QDir>

QString commonPath(
    const QString& path1,
    const QString& path2);

QString assetFilePathUnderContent(
    const QString& contentPath,
    const QString& processedPath,
    const QString& filePath);

QString assetFilePathUnderProcessed(
    const QString& contentPath,
    const QString& processedPath,
    const QString& filePath);

