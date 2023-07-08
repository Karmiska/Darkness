#include "engine/graphics/vulkan/VulkanShaderBinary.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/Device.h"
#include "tools/Recompile.h"
#include "tools/Debug.h"

#include <iostream>
#include <fstream>

namespace engine
{
    namespace implementation
    {
        ShaderBinaryImplVulkan::ShaderBinaryImplVulkan(
            const Device& device,
            const engine::string& binaryPath,
            const engine::string& supportPath,
            int permutationId,
            const engine::vector<engine::string>& defines,
            platform::FileWatcher& watcher)
            : m_device{ &device }
            , m_shaderSupport{ supportPath }
            , m_permutationId{ permutationId }
            , m_defines{ defines }
            , m_watchHandle{ watcher.addWatch(m_shaderSupport.file, [this](const engine::string& changedPath)->engine::string { return this->onFileChange(changedPath); }) }
            , m_change{}
        {
            readFile(binaryPath);
            createShaderModule(device, m_data);
        }

        engine::string ShaderBinaryImplVulkan::onFileChange(const engine::string& /*path*/)
        {
            auto res = recompile(m_shaderSupport, m_permutationId, m_defines);
            readFile(m_shaderSupport.binaryFile);
            createShaderModule(*m_device, m_data);
            for (auto&& change : m_change)
            {
                change.second();
            }
            return res;
        }

        void ShaderBinaryImplVulkan::registerForChange(void* client, std::function<void(void)> change) const
        {
            m_change[client] = change;
        }

        void ShaderBinaryImplVulkan::unregisterForChange(void* client) const
        {
            m_change.erase(client);
        }

        void ShaderBinaryImplVulkan::readFile(const engine::string& path)
        {
            const char* temp = path.data();
            ASSERT(temp);
            std::ifstream shaderFile;
            shaderFile.open(path, std::ios::in | std::ios::binary);
            if (shaderFile.is_open())
            {
                auto begin = shaderFile.tellg();
                shaderFile.seekg(0, std::ios::end);
                auto end = shaderFile.tellg();
                std::size_t size = static_cast<std::size_t>(end - begin);

                m_data.resize(size / 4);
                shaderFile.seekg(0, std::ios::beg);
                shaderFile.read(reinterpret_cast<char*>(m_data.data()), end - begin);
                shaderFile.close();
            }
        }

        const engine::vector<uint32_t> ShaderBinaryImplVulkan::data() const
        {
            return m_data;
        }

        void ShaderBinaryImplVulkan::createShaderModule(const Device& device, const engine::vector<uint32_t>& data)
        {
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = data.size() * 4;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

            m_shaderBinary.reset(new VkShaderModule(), [&](VkShaderModule* module) 
            {
                vkDestroyShaderModule(static_cast<const DeviceImplVulkan*>(device.native())->device(), *module, nullptr);
                delete module;
            });
            auto result = vkCreateShaderModule(static_cast<const DeviceImplVulkan*>(device.native())->device(), &createInfo, nullptr, m_shaderBinary.get());
            ASSERT(result == VK_SUCCESS);
        }

        const VkShaderModule& ShaderBinaryImplVulkan::native() const
        {
            return *m_shaderBinary;
        }
    }
}
