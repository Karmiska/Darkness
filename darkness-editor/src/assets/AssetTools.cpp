#include "AssetTools.h"
#include "tools/PathTools.h"

const QString commonParent(const QString &path1, const QString &path2)
{
    QString ret = path2;

    while (!path1.startsWith(ret))
        ret.chop(1);

    if (ret.isEmpty())
        return ret;

    while (!ret.endsWith(QDir::separator()))
        ret.chop(1);

    return ret;
}

QString commonPath(
    const QString& path1,
    const QString& path2)
{
    QString nativeContentPath = QDir::toNativeSeparators(path1);
    QString nativeProcessedPath = QDir::toNativeSeparators(path2);
    return commonParent(nativeContentPath, nativeProcessedPath);
}

QString assetFilePathUnderContent(
    const QString& contentPath,
    const QString& processedPath,
    const QString& filePath)
{
    QString nativeContentPath = QDir::toNativeSeparators(contentPath);
    QString nativeProcessedPath = QDir::toNativeSeparators(processedPath);
    QString commPath = commonParent(nativeContentPath, nativeProcessedPath);

    QString contentPart = nativeContentPath.right(nativeContentPath.length() - commPath.length());

    QString assetPathUnderContent = filePath.right(filePath.length() - commPath.length() - contentPart.length());
    return commPath + contentPart + assetPathUnderContent;
}

QString assetFilePathUnderProcessed(
    const QString& contentPath,
    const QString& processedPath,
    const QString& filePath)
{
    QString nativeContentPath = QDir::toNativeSeparators(contentPath);
    QString nativeProcessedPath = QDir::toNativeSeparators(processedPath);
    QString nativeFilePath = QDir::toNativeSeparators(filePath);
    QString commPath = commonParent(nativeContentPath, nativeProcessedPath);

    bool alreadyInProcessed = nativeFilePath.startsWith(nativeProcessedPath);
    if (alreadyInProcessed)
        return filePath;

    QString contentPart = nativeContentPath.right(nativeContentPath.length() - commPath.length());
    QString processedPathInternal = nativeProcessedPath.right(nativeProcessedPath.length() - commPath.length());

    QString assetPathUnderContent = filePath.right(filePath.length() - commPath.length() - contentPart.length());
    QString assetPathUnderProcessed = commPath + processedPathInternal + assetPathUnderContent;

    return assetPathUnderProcessed;
}
