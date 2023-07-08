#pragma once

#include "engine/resources/ResourceCommon.h"
#include "containers/string.h"
#include "containers/vector.h"

namespace engine
{
    struct CopyJob
    {
        engine::string from;
        engine::string to;
    };

    struct ExpandedFilePath
    {
        engine::string absolutePath;
        engine::string relativePath;
    };

    class ResourceDropHandler
    {
    public:
        ResourceDropHandler(const engine::string& contentPath, const engine::string& processedPath);
        ProcessResourcePackage handleDrop(
			const engine::vector<engine::string>& sourceFilepaths, 
			const engine::string& destinationPath, 
			unsigned int preferredEncodingFormat, 
			bool generateMips, 
			bool flipNormal,
			bool alphaClipped,
			bool performCopy = true);
    private:
        engine::string m_contentPath;
        engine::string m_processedPath;
        engine::string m_commonAncestor;

        engine::vector<ExpandedFilePath> expandFilepaths(
            const engine::vector<engine::string>& sourceFilepaths,
            const engine::string& destinationPath);
        engine::vector<engine::string> recursiveGather(const engine::string& path);
        vector<ExpandedFilePath> filterDDSFiles(const vector<ExpandedFilePath>& sourceFilepaths);
        engine::vector<CopyJob> copyJobs(const engine::vector<ExpandedFilePath>& sourceFilepaths);

        bool isAssetUnderContent(const engine::string& path);
        
        engine::string pathUnderContent(const ExpandedFilePath& path);
        engine::string pathUnderProcessed(const ExpandedFilePath& path);
        
        void performCopys(const engine::vector<CopyJob>& jobs);
    };
}
