#include "GaussianBlur.vs.h"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/Sampler.h"
#include "tools/ByteRange.h"
#include "tools/Debug.h"
#include <memory>

namespace engine
{
    namespace shaders
    {
#pragma warning( push )
#pragma warning( disable : 4702 )
        std::shared_ptr<const ShaderBinary> GaussianBlurVS::load(const Device& device, ShaderStorage& storage) const
        {
            
            return storage.loadShader(device, "C:/work/darkness/darkness-engine/data/shaders/vulkan/core/tools/GaussianBlur.vs.spv", "C:/work/darkness/darkness-engine/data/shaders/vulkan/core/tools/GaussianBlur.vs.support", -1, {});
            
            ASSERT(false, "Could not load the permutation necessary. This is a bug.");
            return {};
        }
#pragma warning( pop )

        GaussianBlurVS::GaussianBlurVS()
            : m_constantRange{
            
            }
            , m_inputParameters
            {
            
            ShaderInputParameter{"id", "SV_VertexID", "uint"}
            
            
            }
        {}

#pragma warning( push )
#pragma warning( disable : 4100 )
        GaussianBlurVS::GaussianBlurVS(const GaussianBlurVS& cl)
            : m_constantRange{
            
            }
        {
            for (int i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = cl.m_constantRange[i].buffer;
            }

            

            

            

            

            

            

            

            

            

            

        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        GaussianBlurVS::GaussianBlurVS(GaussianBlurVS&& cl)
            : m_constantRange{
            
            }
        {
            for (int i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = std::move(cl.m_constantRange[i].buffer);
            }

            

            

            

            

            

            

            

            

            

            

        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        GaussianBlurVS& GaussianBlurVS::operator=(const GaussianBlurVS& cl)
        {
            for (int i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = cl.m_constantRange[i].buffer;
            }

            

            

            

            

            

            

            

            

            

            

            return *this;
        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        GaussianBlurVS& GaussianBlurVS::operator=(GaussianBlurVS&& cl)
        {
            for (int i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = std::move(cl.m_constantRange[i].buffer);
            }

            

            

            

            

            

            

            

            

            

            

            return *this;
        }
#pragma warning( pop )

        std::vector<std::string> GaussianBlurVS::textureSrvNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::textureUavNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::bufferSrvNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::bufferUavNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::samplerNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::srvNames() const
        {
            return {
                
            };
        }

        std::vector<std::string> GaussianBlurVS::uavNames() const
        {
            return {
                
            };
        }

#pragma warning( push )
#pragma warning( disable : 4100 )
        engine::ResourceDimension GaussianBlurVS::textureDimension(const std::string& name) const
        {
            
            return engine::ResourceDimension::Unknown;
        }
#pragma warning( pop )

        std::vector<TextureSRV> GaussianBlurVS::texture_srvs() const
        {
            std::vector<TextureSRV> result;
            
            return result;
        }

        std::vector<TextureUAV> GaussianBlurVS::texture_uavs() const
        {
            std::vector<TextureUAV> result;
            
            return result;
        }

        std::vector<BufferSRV> GaussianBlurVS::buffer_srvs() const
        {
            std::vector<BufferSRV> result;
            
            return result;
        }

        std::vector<BufferUAV> GaussianBlurVS::buffer_uavs() const
        {
            std::vector<BufferUAV> result;
            
            return result;
        }

        std::vector<TextureBindlessSRV> GaussianBlurVS::bindless_texture_srvs() const
        {
            std::vector<TextureBindlessSRV> result;
            
            return result;
        }

        std::vector<TextureBindlessUAV> GaussianBlurVS::bindless_texture_uavs() const
        {
            std::vector<TextureBindlessUAV> result;
            
            return result;
        }

        std::vector<BufferBindlessSRV> GaussianBlurVS::bindless_buffer_srvs() const
        {
            std::vector<BufferBindlessSRV> result;
            
            return result;
        }

        std::vector<BufferBindlessUAV> GaussianBlurVS::bindless_buffer_uavs() const
        {
            std::vector<BufferBindlessUAV> result;
            
            return result;
        }

        std::vector<Shader::ConstantRange>& GaussianBlurVS::constants()
        {
            return m_constantRange;
        }

        std::vector<Sampler> GaussianBlurVS::samplers() const
        {
            std::vector<Sampler> result;
            
            return result;
        }

        const std::vector<ShaderInputParameter>& GaussianBlurVS::inputParameters() const
        {
            return m_inputParameters;
        }

// warning C4172: returning address of local variable or temporary
// this will never happen as the name will always match the correct resource
#pragma warning( push )
#pragma warning( disable : 4172 )
#pragma warning( disable : 4100 )

        bool GaussianBlurVS::hasTextureSrv(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasTextureUav(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBufferSrv(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBufferUav(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBindlessTextureSrv(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBindlessTextureUav(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBindlessBufferSrv(const std::string& name) const
        {
            
            return false;
        }

        bool GaussianBlurVS::hasBindlessBufferUav(const std::string& name) const
        {
            
            return false;
        }

        const TextureSRV& GaussianBlurVS::textureSrv(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return TextureSRV();
        }

        const TextureUAV& GaussianBlurVS::textureUav(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return TextureUAV();
        }

        const BufferSRV& GaussianBlurVS::bufferSrv(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return BufferSRV();
        }

        const BufferUAV& GaussianBlurVS::bufferUav(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return BufferUAV();
        }

        void GaussianBlurVS::textureSrv(const std::string& name, TextureSRV& texture)
        {
            
            ASSERT(false, "Tried to set non-existing resource");
        }

        void GaussianBlurVS::textureUav(const std::string& name, TextureUAV& texture)
        {
            
            ASSERT(false, "Tried to set non-existing resource");
        }

        void GaussianBlurVS::bufferSrv(const std::string& name, BufferSRV& buffer)
        {
            
            ASSERT(false, "Tried to set non-existing resource");
        }

        void GaussianBlurVS::bufferUav(const std::string& name, BufferUAV& buffer)
        {
            
            ASSERT(false, "Tried to set non-existing resource");
        }

        const Sampler& GaussianBlurVS::sampler(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return Sampler();
        }

        const TextureBindlessSRV& GaussianBlurVS::bindlessTextureSrv(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return TextureBindlessSRV();
        }

        const TextureBindlessUAV& GaussianBlurVS::bindlessTextureUav(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return TextureBindlessUAV();
        }

        const BufferBindlessSRV& GaussianBlurVS::bindlessBufferSrv(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return BufferBindlessSRV();
        }

        const BufferBindlessUAV& GaussianBlurVS::bindlessBufferUav(const std::string& name) const
        {
            
            ASSERT(false, "Tried to look for non-existing resource");
            return BufferBindlessUAV();
        }

#pragma warning( pop )

        uint32_t GaussianBlurVS::descriptorCount() const
        {
            return 0;
        }
    }
}