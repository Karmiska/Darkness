#pragma once

#include "containers/vector.h"
#include "ResClientCommon.h"

namespace resource_client
{
    class ModelSplitter
    {
    public:
        engine::vector<SplitTask> splitModelTask(Task& container);
        TaskResult joinModelTask(engine::vector<SplitTaskResult>& results);
    };
}
