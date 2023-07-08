#pragma once

#include "engine/graphics/Format.h"
#include "containers/string.h"
#include <functional>
#include <map>

namespace engine
{
    bool isModelFormat(const engine::string& filepath);
    bool isImageFormat(const engine::string& filepath);
    bool isPrefabFormat(const engine::string& filepath);

    enum class AssetTask
    {
        ImageRecompress
    };

    struct AssetNotification
    {
        AssetTask task;
        engine::string sourcePath;
        engine::string destinationPath;
        engine::Format encodeType;
    };

    class AssetNotificationService
    {
    public:
        void registerListener(void* client, std::function<void(const AssetNotification&)> notification);
        void unregisterListener(void* client);

        void sendNotification(const AssetNotification& notification);

    private:
        std::map<void*, std::function<void(const AssetNotification&)>> m_clients;
    };

    extern AssetNotificationService GlobalAssetNotificationService;
}
