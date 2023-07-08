#pragma once

#include "engine/graphics/ShaderBinaryImplIf.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/ShaderSupport.h"
#include "platform/FileWatcher.h"
#include "containers/memory.h"
#include "containers/string.h"
#include "containers/vector.h"
#include <map>

namespace engine
{
    class Device;

    namespace implementation
    {
        class ShaderBinaryImplVulkan : public ShaderBinaryImplIf
        {
        public:
            ShaderBinaryImplVulkan(
                const Device& device, 
                const engine::string& binaryPath,
                const engine::string& supportPath,
                int permutationId,
                const engine::vector<engine::string>& defines,
                platform::FileWatcher& watcher);

            void registerForChange(void* client, std::function<void(void)> change) const;
            void unregisterForChange(void* client) const;

            const VkShaderModule& native() const;
            const engine::vector<uint32_t> data() const;
        private:
            const Device* m_device;
            engine::shared_ptr<VkShaderModule> m_shaderBinary = nullptr;
            engine::vector<uint32_t> m_data;
            
            ShaderSupport m_shaderSupport;
            int m_permutationId;
            engine::vector<engine::string> m_defines;

            platform::WatchHandle m_watchHandle;
            mutable std::map<void*, std::function<void(void)>> m_change;

            engine::string onFileChange(const engine::string& path);

            void readFile(const engine::string& path);
            void createShaderModule(const Device& device, const engine::vector<uint32_t>& data);
        };
    }
}

