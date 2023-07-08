#pragma once

#include "containers/vector.h"
#include "ResClientCommon.h"

namespace resource_client
{
    class ImageSplitter
    {
    public:
        ImageSplitter();
        ~ImageSplitter();
        engine::vector<SplitTask> splitImageTask(Task& container);
        TaskResult joinImageTask(engine::vector<SplitTaskResult>& results);
    };
}
