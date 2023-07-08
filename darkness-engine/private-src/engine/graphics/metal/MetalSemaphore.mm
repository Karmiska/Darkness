#include "engine/graphics/metal/MetalSemaphore.h"
#include "engine/graphics/metal/MetalHeaders.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        SemaphoreImpl::SemaphoreImpl(const Device& device)
        : m_semaphore{ nullptr }
        {
            /*VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            auto result = vkCreateSemaphore(
                                            DeviceImplGet::impl(device).device(),
                                            &semaphoreInfo,
                                            nullptr,
                                            m_semaphore.get());
            ASSERT(result == VK_SUCCESS);*/
        }
        
        MetalSemaphore& SemaphoreImpl::native()
        {
            return *m_semaphore;
        }
        
        const MetalSemaphore& SemaphoreImpl::native() const
        {
            return *m_semaphore;
        }
    }
}
