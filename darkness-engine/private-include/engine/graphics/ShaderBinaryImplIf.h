#pragma once

#include <functional>

namespace engine
{
    namespace implementation
    {
        class ShaderBinaryImplIf
        {
        public:
            virtual ~ShaderBinaryImplIf() {};
            
            virtual void registerForChange(void* client, std::function<void(void)> change) const = 0;
            virtual void unregisterForChange(void* client) const = 0;
        };
    }
}
