#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include <functional>

namespace engine
{
    enum class ResourceType
    {
        Image,
        Model
    };

    struct ProcessResourceItem
    {
        ResourceType type;
        unsigned int encodingFormat;
        bool generateMips;
		bool flipNormal;
		bool alphaClipped;
        engine::string absoluteSourceFilepath;
        engine::string absoluteContentFilepath;
        engine::string absoluteProcessedFilepath;
    };

    using ResourceProcessingStarted = std::function<void(ProcessResourceItem)>;
    using ResourceProcessingFinished = std::function<void(ProcessResourceItem)>;
    using ResourceProcessingProgress = std::function<void(ProcessResourceItem, float)>;
    using ResourceProcessingProgressMessage = std::function<void(ProcessResourceItem, float, const engine::string&)>;

    struct ProcessResourcePackage
    {
        engine::vector<ProcessResourceItem> images;
        engine::vector<ProcessResourceItem> models;

        ResourceProcessingStarted onStarted;
        ResourceProcessingFinished onFinished;
        ResourceProcessingProgress onProgress;
        ResourceProcessingProgressMessage onProgressMessage;
    };
}
