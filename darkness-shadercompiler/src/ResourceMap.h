#pragma once

#include "containers/string.h"
#include "containers/vector.h"

namespace shadercompiler
{
    enum class ResourceType
    {
        Texture,
        Buffer,
        Sampler,
        RootConstant,
        AccelerationStructure,
        Function
    };

    enum class ResourceAccess
    {
        Read,
        Write
    };

    struct InputParameter
    {
        engine::string name;
        engine::string type;
        engine::string semantic;
    };

    struct ResourceMapping
    {
        engine::string name;
        ResourceType type;
        ResourceAccess access;
        engine::string templatedType;
        bool bindless;
        int bindingIndex;
        engine::string dimension;
        int lineNumber;
        bool structured;
        engine::vector<std::pair<engine::string, engine::string>> constants;
    };

    class ResourceMap
    {
    public:
        ResourceMap(engine::vector<engine::string>& lines);

        engine::vector<ResourceMapping>& textureSRV() { return m_textureSRV; }
        engine::vector<ResourceMapping>& textureUAV() { return m_textureUAV; }
        engine::vector<ResourceMapping>& bufferSRV() { return m_bufferSRV; }
        engine::vector<ResourceMapping>& bufferUAV() { return m_bufferUAV; }
        engine::vector<ResourceMapping>& bindlessTextureSRV() { return m_bindlessTextureSRV; }
        engine::vector<ResourceMapping>& bindlessTextureUAV() { return m_bindlessTextureUAV; }
        engine::vector<ResourceMapping>& bindlessBufferSRV() { return m_bindlessBufferSRV; }
        engine::vector<ResourceMapping>& bindlessBufferUAV() { return m_bindlessBufferUAV; }
        engine::vector<ResourceMapping>& accelerationStructures() { return m_accelerationStructures; }
        engine::vector<ResourceMapping>& samplers() { return m_samplers; }
        engine::vector<ResourceMapping>& rootConstants() { return m_rootConstants; }
        engine::vector<ResourceMapping>& constantBuffers() { return m_constants; }
        engine::vector<InputParameter>& inputParameters() { return m_inputParameters; }
    private:
        engine::vector<ResourceMapping> m_textureSRV;
        engine::vector<ResourceMapping> m_textureUAV;
        engine::vector<ResourceMapping> m_bufferSRV;
        engine::vector<ResourceMapping> m_bufferUAV;
        engine::vector<ResourceMapping> m_bindlessTextureSRV;
        engine::vector<ResourceMapping> m_bindlessTextureUAV;
        engine::vector<ResourceMapping> m_bindlessBufferSRV;
        engine::vector<ResourceMapping> m_bindlessBufferUAV;
        engine::vector<ResourceMapping> m_accelerationStructures;
        engine::vector<ResourceMapping> m_samplers;
        engine::vector<ResourceMapping> m_rootConstants;
        engine::vector<ResourceMapping> m_constants;
        engine::vector<InputParameter> m_inputParameters;

        struct StructDef
        {
            engine::string name;
            engine::vector<InputParameter> param;
        };
        engine::vector<StructDef> m_knownStructures;
    };
}