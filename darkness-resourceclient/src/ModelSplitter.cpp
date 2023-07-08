#include "ModelSplitter.h"
#include "platform/Uuid.h"

using namespace engine;

namespace resource_client
{
    engine::vector<SplitTask> ModelSplitter::splitModelTask(Task& container)
    {
        engine::vector<SplitTask> result;
        SplitTask task;
        task.type = TaskType::Model;

        task.model.modelTargetPath = container.model.modelTargetPath;
        task.model.assetName = container.model.assetName;
        task.model.scale = container.model.scale;
        task.model.rotation = container.model.rotation;

        task.data = move(container.data);
        task.progress = 0.0f;
        task.bytes = task.data.size();
        task.subTaskId = platform::uuid();
        task.taskId = container.taskId;
        result.emplace_back(std::move(task));
        return result;
    }

    TaskResult ModelSplitter::joinModelTask(engine::vector<SplitTaskResult>& results)
    {
        ASSERT(results.size() == 1, "Multipart model messages unsupported");
        TaskResult result;
        result.model.modelData = std::move(results[0].model.modelData);
        result.model.prefabData = std::move(results[0].model.prefabData);
        result.type = TaskType::Model;
        return result;
    }
}
