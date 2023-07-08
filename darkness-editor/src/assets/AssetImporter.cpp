#include "AssetImporter.h"
#include "tools/Debug.h"
#include "tools/image/ImageIf.h"
#include "tools/image/Image.h"
#include "tools/PathTools.h"

#include "containers/memory.h"
#include <QFile>
#include <cctype>

using namespace engine;
using namespace engine::image;

AssetImporter::AssetImporter(
    const QString& contentPath,
    const QString& processedAssetsPath)
    : m_contentPath{ contentPath }
    , m_processedAssetPath{ processedAssetsPath }
    , m_dropHandler{ engine::string(contentPath.toStdString().c_str()), engine::string(processedAssetsPath.toStdString().c_str()) }
    , m_resourceHost{}
{
}

AssetImporter::~AssetImporter()
{
}

engine::vector<ProcessResourceItem> filterAllButAccepted(engine::vector<ProcessResourceItem>& list, const engine::vector<engine::string>& extensions)
{
    engine::vector<ProcessResourceItem> result;
    for (auto&& item : list)
    {
        bool hit = false;
        auto itemExt = pathExtractExtension(item.absoluteSourceFilepath);
        std::transform(itemExt.begin(), itemExt.end(), itemExt.begin(),
            [](unsigned char c) { return std::tolower(c); });

        for (auto&& ext : extensions)
        {
            if (itemExt == ext)
            {
                hit = true;
                break;
            }
        }
        if (hit)
        {
            result.emplace_back(item);
        }
    }
    return result;
}

void AssetImporter::processItems(
    const QList<QString>& sourceFilePath,
    const QString& targetPath,
    const engine::Vector3f& scale,
    const engine::Quaternionf& rotation,
    const engine::string& preferredEncoding,
	bool flipNormal,
	bool alphaClipped,
    bool performCopy)
{
    engine::vector<engine::string> sources;
    engine::string destination = targetPath.toStdString().c_str();
    for (auto&& src : sourceFilePath)
    {
        sources.emplace_back(src.toStdString().c_str());
    }
    
    auto package = m_dropHandler.handleDrop(sources, destination, static_cast<int>(Format::BC7_UNORM), true, flipNormal, alphaClipped, performCopy);
    package.models = filterAllButAccepted(package.models, { "obj", "fbx", "lwo", "lxo", "stl" });

    package.onStarted = [&](ProcessResourceItem item)
    {
        //LOG("RESOURCE HOST. Task Started: %s", item.absoluteSourceFilepath.c_str());
        QString path = QString::fromStdString(pathExtractFilename(item.absoluteSourceFilepath).c_str());
        emit assetWorkStarted(path);
    };
    package.onFinished = [&](ProcessResourceItem item)
    {
        //LOG("RESOURCE HOST. Task Finished: %s", item.absoluteSourceFilepath.c_str());
        QString path = QString::fromStdString(pathExtractFilename(item.absoluteSourceFilepath).c_str());
        emit assetWorkStopped(path);
    };
    package.onProgress = [&](ProcessResourceItem item, float progress)
    {
        //LOG("RESOURCE HOST. Task: %s, Progress: %f", item.absoluteSourceFilepath.c_str(), progress);
        QString path = QString::fromStdString(pathExtractFilename(item.absoluteSourceFilepath).c_str());
        emit assetWorkProgress(path, progress);
    };
    package.onProgressMessage = [&](ProcessResourceItem item, float progress, const engine::string& message)
    {
        QString path = QString::fromStdString(pathExtractFilename(item.absoluteSourceFilepath).c_str());
        emit assetWorkStatusChange(path, QString::fromStdString(message.c_str()));
        /*LOG("RESOURCE HOST. Task: %s, Progress: %f, Message: %s",
            item.absoluteSourceFilepath.c_str(),
            progress,
            message.c_str());*/
    };

    m_resourceHost.processResources(package);
}

void AssetImporter::createCubemap(
    const QList<QString>& sourceFilePath,
    const QList<QString>& targetFilePath)
{
    ASSERT(sourceFilePath.size() == 6, "Cubemap create got invalid number of images");

    // reorder images so as to match dx cubemap slice order
    engine::vector<engine::string> sourceFilePathOrdered(6);
    engine::vector<engine::string> targetFilePathOrdered(6);
    for (int i = 0; i < sourceFilePath.size(); ++i)
    {
        auto srcPath = engine::string(sourceFilePath[i].toStdString().c_str());
        auto dstPath = engine::string(targetFilePath[i].toStdString().c_str());

        bool positive = srcPath.find("pos") != engine::string::npos;
        bool negative = srcPath.find("neg") != engine::string::npos;
        bool x = srcPath.find("x") != engine::string::npos;
        bool y = srcPath.find("y") != engine::string::npos;
        bool z = srcPath.find("z") != engine::string::npos;

        if ((positive && negative) || (!positive && !negative) ||
            (!x && !y && !z))
        {
            LOG_WARNING("Could not figure out cubemap side order. filename should contain either 'pos' or 'neg' and 'x', 'y', 'z' characters as part of the filename. Bailing on the ordering.");
            sourceFilePathOrdered.clear();
            targetFilePathOrdered.clear();
            for (auto&& item : sourceFilePath) { sourceFilePathOrdered.emplace_back(item.toStdString().c_str()); }
            for (auto&& item : targetFilePath) { targetFilePathOrdered.emplace_back(item.toStdString().c_str()); }
            break;
        }

        if (positive && x) { sourceFilePathOrdered[0] = srcPath; targetFilePathOrdered[0] = dstPath; }
        if (negative && x) { sourceFilePathOrdered[1] = srcPath; targetFilePathOrdered[1] = dstPath; }
        if (positive && y) { sourceFilePathOrdered[2] = srcPath; targetFilePathOrdered[2] = dstPath; }
        if (negative && y) { sourceFilePathOrdered[3] = srcPath; targetFilePathOrdered[3] = dstPath; }
        if (positive && z) { sourceFilePathOrdered[4] = srcPath; targetFilePathOrdered[4] = dstPath; }
        if (negative && z) { sourceFilePathOrdered[5] = srcPath; targetFilePathOrdered[5] = dstPath; }
    }

    engine::vector<engine::shared_ptr<image::ImageIf>> images;
    for (auto&& src : targetFilePathOrdered)
    {
        images.emplace_back(image::Image::createImage(src, image::ImageType::DDS));
    }

    // test that they're all same sized and have same amount of mips
    int mips = static_cast<int>(images[0]->mipCount());
    int width = static_cast<int>(images[0]->width());
    int height = static_cast<int>(images[0]->height());
    engine::Format format = images[0]->format();
    for(auto&& image : images)
    {
        ASSERT(image->width() == width, "Cubemap images are differing by size (width)");
        ASSERT(image->height() == height, "Cubemap images are differing by size (height)");
        ASSERT(image->mipCount() == mips, "Cubemap images are differing by mip count");
        ASSERT(image->format() == format, "Cubemap images are differing by format");
    }

    // we'll use the parent folder name for the new cubemap filename
    auto filePath = pathExtractFolder(sourceFilePathOrdered[0], false);
    auto newFileName = pathExtractFilename(filePath);
    newFileName += ".dds";
    auto newFilePath = pathJoin(filePath, newFileName);

    // processed path
    auto processedFilePath = pathExtractFolder(targetFilePathOrdered[0], false);
    auto processedNewFilePath = pathJoin(processedFilePath, newFileName);

    auto newImage = image::Image::createImage(newFilePath, image::ImageType::DDS, format, width, height, 6, mips);
    newImage->reserve();
    for (int mip = 0; mip < mips; ++mip)
    {
        auto dstSurfacePosX = newImage->map(mip, 0);
        auto dstSurfaceNegX = newImage->map(mip, 1);
        auto dstSurfacePosY = newImage->map(mip, 2);
        auto dstSurfaceNegY = newImage->map(mip, 3);
        auto dstSurfacePosZ = newImage->map(mip, 4);
        auto dstSurfaceNegZ = newImage->map(mip, 5);

        auto srcSurfacePosX = images[0]->map(mip, 0);
        auto srcSurfaceNegX = images[1]->map(mip, 0);
        auto srcSurfacePosY = images[2]->map(mip, 0);
        auto srcSurfaceNegY = images[3]->map(mip, 0);
        auto srcSurfacePosZ = images[4]->map(mip, 0);
        auto srcSurfaceNegZ = images[5]->map(mip, 0);

        memcpy(const_cast<uint8_t*>(dstSurfacePosX.data), srcSurfacePosX.data, dstSurfacePosX.sizeBytes);
        memcpy(const_cast<uint8_t*>(dstSurfaceNegX.data), srcSurfaceNegX.data, dstSurfaceNegX.sizeBytes);
        memcpy(const_cast<uint8_t*>(dstSurfacePosY.data), srcSurfacePosY.data, dstSurfacePosY.sizeBytes);
        memcpy(const_cast<uint8_t*>(dstSurfaceNegY.data), srcSurfaceNegY.data, dstSurfaceNegY.sizeBytes);
        memcpy(const_cast<uint8_t*>(dstSurfacePosZ.data), srcSurfacePosZ.data, dstSurfacePosZ.sizeBytes);
        memcpy(const_cast<uint8_t*>(dstSurfaceNegZ.data), srcSurfaceNegZ.data, dstSurfaceNegZ.sizeBytes);
    }
    newImage->save();
    QFile::copy(QString::fromStdString(newFilePath.c_str()), QString::fromStdString(processedNewFilePath.c_str()));
}
