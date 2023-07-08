#include "engine/graphics/metal/MetalRootParameter.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalConversions.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        RootParameterImpl::RootParameterImpl()
        {
        }
        
        void RootParameterImpl::binding(unsigned int index)
        {
        }
        
        unsigned int RootParameterImpl::binding() const
        {
            return 0;
        }
        
        void RootParameterImpl::visibility(ShaderVisibility visibility)
        {
        }
        
        const ShaderVisibility RootParameterImpl::visibility() const
        {
            return {};
        }
        
        void RootParameterImpl::descriptorType(DescriptorType type)
        {
        }
        
        DescriptorType RootParameterImpl::descriptorType() const
        {
            return {};
        }
        
        void RootParameterImpl::initAsConstants(unsigned int /*reg*/, unsigned int /*num32BitValues*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::initAsCBV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::initAsSRV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::initAsUAV(unsigned int /*reg*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::initAsDescriptorRange(DescriptorRangeType /*type*/, unsigned int /*reg*/, unsigned int /*count*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::initAsDescriptorTable(unsigned int /*rangeCount*/, ShaderVisibility /*visibility*/)
        {
            // TODO
        }
        
        void RootParameterImpl::setTableRange(unsigned int /*rangeIndex*/, DescriptorRangeType /*type*/, unsigned int /*reg*/, unsigned int /*count*/, unsigned int /*space*/)
        {
            // TODO
        }
    }
}
