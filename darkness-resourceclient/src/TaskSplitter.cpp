#include "TaskSplitter.h"
#include "tools/AssetTools.h"

namespace resource_client
{
    engine::vector<SplitTask> TaskSplitter::taskSplitter(Task& container)
    {
        if (container.type == TaskType::Image)
            return m_imageSplitter.splitImageTask(container);
        else if (container.type == TaskType::Model)
            return m_modelSplitter.splitModelTask(container);
        else
            ASSERT(false, "Unsupported task type");
        return {};
    }

    TaskResult TaskSplitter::taskJoiner(engine::vector<SplitTaskResult>& results)
    {
        if (results.size() == 0)
            return {};

        if (results[0].type == TaskType::Image)
            return m_imageSplitter.joinImageTask(results);
        else if (results[0].type == TaskType::Model)
            return m_modelSplitter.joinModelTask(results); 
        else
            ASSERT(false, "Unsupported task type");
        return {};
    }
}