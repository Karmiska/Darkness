#pragma once

#include "containers/vector.h"
#include "ImageSplitter.h"
#include "ModelSplitter.h"

namespace resource_client
{
    struct Task;
    struct SplitTask;
    struct SplitTaskResult;
    struct TaskResult;

    constexpr int MaximumTaskSize = 256;

    class TaskSplitter
    {
    public:
        engine::vector<SplitTask> taskSplitter(Task& container);
        TaskResult taskJoiner(engine::vector<SplitTaskResult>& results);
    private:
        ImageSplitter m_imageSplitter;
        ModelSplitter m_modelSplitter;
    };
    
}
