#include "engine/resources/ResourceDropHandler.h"
#include "tools/AssetTools.h"
#include "tools/PathTools.h"
#include "platform/File.h"
#include "platform/Directory.h"
#include <cctype>

using namespace engine;

namespace engine
{
    ResourceDropHandler::ResourceDropHandler(const string& contentPath, const string& processedPath)
        : m_contentPath{ contentPath }
        , m_processedPath{ processedPath }
        , m_commonAncestor{ pathCommonAncestor(contentPath, processedPath) }
    {}

    ProcessResourcePackage ResourceDropHandler::handleDrop(
        const vector<string>& sourceFilepaths, 
        const string& destinationPath, 
        unsigned int preferredEncodingFormat,
        bool generateMips,
		bool flipNormal,
		bool alphaClipped,
        bool performCopy)
    {
        ASSERT(isAssetUnderContent(destinationPath), "Can't handle asset drops to folders outside content path");

        // expand folder and files => just source files
        auto expandedFilepaths = expandFilepaths(sourceFilepaths, destinationPath);

        auto destCommon = pathSubtractCommon(m_contentPath, destinationPath);

        // check which of the sources are not under content yet
        // and copy those to content
        auto copy = copyJobs(expandedFilepaths);
        if(performCopy)
            performCopys(copy);

        // copy DDS files directly. we don't want to encode them.
        expandedFilepaths = filterDDSFiles(expandedFilepaths);

        // now we have only content paths
        ProcessResourcePackage result;
        for (auto&& file : expandedFilepaths)
        {
            if(isImageFormat(file.relativePath))
            {
                result.images.emplace_back(ProcessResourceItem{
                    ResourceType::Image,
                    preferredEncodingFormat,
                    generateMips,
					flipNormal,
					alphaClipped,
                    file.absolutePath,
                    pathJoin({ m_contentPath, file.relativePath}),
                    pathJoin({ m_processedPath, file.relativePath }) });
            }
            else if (isModelFormat(file.relativePath))
            {
                result.models.emplace_back(ProcessResourceItem{ 
                    ResourceType::Model, 
                    preferredEncodingFormat,
                    generateMips,
					flipNormal,
					alphaClipped,
                    file.absolutePath, 
                    pathJoin({ m_contentPath, file.relativePath }),
                    pathJoin({ m_processedPath, file.relativePath }) });
            }
        }
        return result;
    }

    vector<ExpandedFilePath> ResourceDropHandler::filterDDSFiles(const vector<ExpandedFilePath>& sourceFilepaths)
    {
        vector<ExpandedFilePath> result;
        for (auto&& source : sourceFilepaths)
        {
            auto ext = pathExtractExtension(source.absolutePath);
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == "dds")
            {
                auto processedPath = pathUnderProcessed(source);
                performCopys({ CopyJob{ source.absolutePath, processedPath } });
            }
            else
                result.emplace_back(source);
        }
        return result;
    }

    vector<CopyJob> ResourceDropHandler::copyJobs(const vector<ExpandedFilePath>& sourceFilepaths)
    {
        vector<CopyJob> result;
        for (auto&& src : sourceFilepaths)
        {
            if (!isAssetUnderContent(src.absolutePath))
            {
                result.emplace_back(CopyJob{ src.absolutePath, pathUnderContent(src) });
            }
        }
        return result;
    }

    bool ResourceDropHandler::isAssetUnderContent(const engine::string& path)
    {
        string pathFolder = "";
        if (pathIsFile(path))
            pathFolder = pathExtractFolder(path);
        else
            pathFolder = path;

        if (pathRemoveTrailingDelimiter(pathFolder) == pathRemoveTrailingDelimiter(m_contentPath))
            return true;

        auto ancestor = pathCommonAncestor(pathRemoveTrailingDelimiter(pathFolder), pathRemoveTrailingDelimiter(m_contentPath));
        return pathSplit(ancestor).size() >= pathSplit(m_contentPath).size();
    }

    engine::string ResourceDropHandler::pathUnderContent(const ExpandedFilePath& path)
    {
        return pathJoin(m_contentPath, path.relativePath);
    }

    engine::string ResourceDropHandler::pathUnderProcessed(const ExpandedFilePath& path)
    {
        return pathJoin(m_processedPath, path.relativePath);
    }

    vector<ExpandedFilePath> ResourceDropHandler::expandFilepaths(
        const vector<string>& sourceFilepaths,
        const engine::string& destinationPath)
    {
        auto destCommon = pathSubtractCommon(m_contentPath, destinationPath);
        vector<ExpandedFilePath> result;
        for (auto&& src : sourceFilepaths)
        {
            if (pathIsFolder(src))
            {
                auto paths = recursiveGather(src);
                for (auto p : paths)
                {
                    auto foldername = pathExtractFolderName(src);
                    auto common = pathSubtractCommon(src, p);
                    result.emplace_back(ExpandedFilePath{ p, pathJoin(destCommon, pathJoin(foldername, common)) });
                }
            }
            else
            {
                result.emplace_back(ExpandedFilePath{ src, pathJoin(destCommon, pathExtractFilename(src)) });
            }
        }
        return result;
    }

    vector<string> ResourceDropHandler::recursiveGather(const string& path)
    {
        ASSERT(pathIsFolder(path), "Tried to gather from file instead of folder");

        vector<string> result;
        Directory directory(path);
        for (auto&& dir : directory.directories())
        {
            auto contents = recursiveGather(pathJoin(path, dir));
            result.insert(result.end(), contents.begin(), contents.end());
        }
        auto files = directory.files();
        for (auto&& file : files)
        {
            result.emplace_back(pathJoin(path, file));
        }
        return result;
    }

    void ResourceDropHandler::performCopys(const engine::vector<CopyJob>& jobs)
    {
        for (auto&& job : jobs)
        {
            // check for/create path
            if (!pathExists(pathExtractFolder(job.to)))
            {
                Directory dir(pathExtractFolder(job.to));
                dir.create();
            }
            
            // delete existing file
            if (fileExists(job.to))
                fileDelete(job.to);

            // copy the new file
            fileCopy(job.from, job.to);
        }
    }
}
