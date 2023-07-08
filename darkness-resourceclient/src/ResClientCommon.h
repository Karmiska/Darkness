#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Quaternion.h"
#include "platform/Environment.h"

namespace engine
{
    class Process;
}

namespace resource_client
{
#ifdef _DEBUG
    static const char* ResourceTaskPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\debug\\DarknessResourceTask.exe";
    static const char* ResourceTaskWorkingPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\debug";

    static const char* ResourceTaskFallbackPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\release\\DarknessResourceTask.exe";
    static const char* ResourceTaskFallbackWorkingPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\release";
#else
    static const char* ResourceTaskPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\release\\DarknessResourceTask.exe";
    static const char* ResourceTaskWorkingPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\release";

    static const char* ResourceTaskFallbackPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\debug\\DarknessResourceTask.exe";
    static const char* ResourceTaskFallbackWorkingPath = "..\\..\\..\\..\\darkness-resourcetask\\bin\\win64\\debug";
#endif

    static const char* ResourceTaskArgumentsIp = "--ip=";
    static const char* ResourceTaskArgumentsPort = " --port=";
    static const char* ResourceTaskArgumentsId = " --id=";
    static const char* ResourceTaskArgumentsHostId = " --hostid=";
    

    enum class TaskType
    {
        Image,
        Model
    };

    struct TaskImage
    {
        engine::string extension;
        int32_t format;
        bool generateMips;
		bool flipNormal;
		bool alphaClipped;
    };

    struct TaskModel
    {
        engine::string modelTargetPath;
        engine::string assetName;
        engine::Vector3f scale;
        engine::Quaternionf rotation;
    };

    struct Task
    {
        engine::string hostId;
        engine::string hostTaskId;
        engine::string taskId;

        TaskType type;
        TaskImage image;
        TaskModel model;

        engine::vector<char> data;

        float lastProgressUpdateValue = -1.0f;
        engine::string lastProgressMessageUpdateValue = "";
    };

    struct TaskImageResult
    {
        engine::vector<char> data;
        uint32_t width;
        uint32_t height;
        uint32_t mips;
        int32_t format;
    };

    struct TaskModelResult
    {
        engine::vector<char> modelData;
        engine::vector<char> prefabData;
    };

    struct TaskResult
    {
        TaskImageResult image;
        TaskModelResult model;
        TaskType type;
        engine::string id;
        engine::string hostid;
    };

    struct Worker
    {
        bool clientReady;
        engine::string id;
        engine::shared_ptr<engine::Process> process;
    };

    struct SplitTaskImageResult
    {
        int32_t width;
        int32_t height;
        int32_t format;
		int32_t mipCount;
		int32_t mip;
		int32_t partId;
		int32_t partWidth;
		int32_t partHeight;
        engine::vector<char> data;
    };

    struct SplitTaskModelResult
    {
        engine::vector<char> modelData;
        engine::vector<char> prefabData;
    };

    struct SplitTaskResult
    {
        TaskType type;
        SplitTaskImageResult image;
        SplitTaskModelResult model;
    };

    struct SplitImageTask
    {
        int32_t width;
        int32_t height;
        int32_t stride;
        int32_t sourceFormat;
        int32_t targetFormat;
        int32_t originalFormat;
		int32_t mipCount;
		int32_t mip;
		int32_t partId;
		int32_t partWidth;
		int32_t partHeight;
    };

    struct SplitModelTask
    {
        engine::string modelTargetPath;
        engine::string assetName;
        engine::Vector3f scale;
        engine::Quaternionf rotation;
    };

    struct SplitTask
    {
        bool requestedWorker = false;
        bool returnedWorker = false;
        bool processing = false;
        bool done = false;
        float progress = 0.0f;
        engine::string progressMessage = "";
        uint64_t bytes;
        Worker worker;
        SplitTaskResult result;
        TaskType type;

        engine::string taskId;
        engine::string subTaskId;
        SplitImageTask image;
        SplitModelTask model;
        engine::vector<char> data;
    };

}
