#pragma once

#include <QObject>
#include <QString>
#include <QMutex>
#include <QRunnable>

#include "containers/string.h"

//#include "tools/image/Image.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Quaternion.h"

#include "engine/resources/ResourceDropHandler.h"
#include "engine/resources/ResourceHost.h"

/*class AssetImporterWorker : public QObject,
                            public QRunnable
{
    Q_OBJECT
public:
    AssetImporterWorker(
        engine::ResourceHost& resourceHost,
        QMutex& imageMutex,
        QMutex& compressMutex,
        const QString& contentPath,
        const QString& processedAssetsPath,
        const QString& sourceFilePath,
        const QString& targetPath,
        const engine::Vector3f& scale,
        const engine::Quaternionf& rotation,
        const engine::string& preferredEncoding,
        bool performCopy);

    void run() override;
    void reportProgress(float progress, const char* filename);
signals:
    void assetWorkStarted(QString sourceFilePath) const;
    void assetWorkStatusChange(QString sourceFilePath, QString msg) const;
    void assetWorkProgress(QString sourceFilePath, float progress) const;
    void assetWorkStopped(QString sourceFilePath, const AssetImporterWorker* worker) const;

private:
    void copyAsset(
        const QString& sourcePath,
        const QString& destinationPath,
        bool performCopy) const;

    void processAsset(
        const QString& sourcePath,
        const QString& destinationPath,
        const QString& contentPath,
        const QString& reprocessSourcePath,
        const QString& reprocessDestinationPath,
        bool performCopy) const;

    void processModel(
        const QString& sourcePath,
        const QString& destinationPath,
        const QString& contentPath) const;

    void processImage(
        const QString& sourcePath,
        const QString& destinationPath,
        const QString& reprocessSourcePath,
        const QString& reprocessDestinationPath,
        bool performCopy,
        engine::Format encodeType = engine::Format::BC1_UNORM) const;
private:
    engine::ResourceHost& m_resourceHost;
    QMutex& m_imageMutex;
    QMutex& m_compressMutex;
    QString m_contentPath;
    QString m_processedAssetPath;
    QString m_sourceFilePath;
    QString m_targetPath;
    engine::Vector3f m_scale;
    engine::Quaternionf m_rotation;
    engine::string m_preferredEncoding;
    bool m_performCopy;
};*/

class AssetImporter : public QObject
{
    Q_OBJECT
public:
    AssetImporter(
        const QString& contentPath,
        const QString& processedAssetsPath);
    virtual ~AssetImporter();
public slots:
    void processItems(
        const QList<QString>& sourceFilePaths,
        const QString& targetPath,
        const engine::Vector3f& scale,
        const engine::Quaternionf& rotation,
        const engine::string& preferredEncoding,
		bool flipNormal,
		bool alphaClipped,
        bool performCopy);
    void createCubemap(
        const QList<QString>& sourceFilePath,
        const QList<QString>& targetFilePath);

    /*void onAssetWorkStarted(QString sourceFilePath) const;
    void onAssetWorkStatusChange(QString sourceFilePath, QString msg) const;
    void onAssetWorkProgress(QString sourceFilePath, float progress) const;
    void onAssetWorkStopped(QString sourceFilePath, const AssetImporterWorker* worker) const;*/

signals:
    void assetWorkStarted(QString sourceFilePath) const;
    void assetWorkStatusChange(QString sourceFilePath, QString msg) const;
    void assetWorkProgress(QString sourceFilePath, float progress) const;
    void assetWorkStopped(QString sourceFilePath) const;


private:
    QString m_contentPath;
    QString m_processedAssetPath;
    QMutex m_workerImageMutex;
    QMutex m_workerCompressMutex;

    engine::ResourceDropHandler m_dropHandler;
    engine::ResourceHost m_resourceHost;
};
