#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "ResourceMap.h"
#include "containers/memory.h"
#include "Preprocessor.h"
#include <mutex>
#include <filesystem>

namespace shadercompiler
{
    struct PermutationItem
    {
        engine::string type;
        engine::string variableName;
        engine::string value;
        engine::string flag;
    };

    struct Permutation
    {
        engine::vector<PermutationItem> list;
        engine::string id;
        engine::vector<engine::string> defines;
    };

    struct ShaderPipelineStageItem
    {
        engine::shared_ptr<shadercompiler::ResourceMap> resources;
        engine::vector<engine::string> recursiveContent;
        engine::string permutationPath;
        engine::string sourcePath;

        struct VulkanBinding
        {
            int lineNumber;
            int bindingNumber;
            int setNumber;
        };
        engine::vector<VulkanBinding> vulkanBindings;
    };

    struct ShaderPipelineStage
    {
        engine::string name;     // SomeShader
        engine::string filename; // source filepath C:\..\SomeShader.cs.hlsl

        // with OPTION_DEBUG =   debug, OPTION_DEBUG
        engine::vector<Condition> options;

        // with ENUM_COLOR_RED =   COLOR               red, ENUM_COLOR_RED
        engine::unordered_map<engine::string, engine::vector<Condition>> enums;

        int nonBindlessSetCount() const;
        int setStart = 0;
        int setCount() const;

        // permutations from the source file
        engine::vector<Permutation> permutations;

        engine::shared_ptr<std::mutex> itemMutex;
        engine::vector<ShaderPipelineStageItem> items;

        std::filesystem::file_time_type sourceTimestamp;
        std::filesystem::file_time_type binaryTimestamp;
        std::filesystem::file_time_type codeInterfacesTimestamp;

        engine::vector<engine::string> shaderIncludeDepencyPaths;

        bool buildShaderBinary;
        bool buildCodeInterface;
    };

    struct ShaderPipelineConfiguration
    {
        engine::string path;
        engine::string pipelineName;
        bool buildCodeInterface;
        engine::vector<ShaderPipelineStage> stages;
    };

    struct ShaderLocatorParameters
    {
        const engine::string& shaderRootPath;
        const engine::string& shaderBinaryPathDX12;
        const engine::string& shaderBinaryPathVulkan;
        const engine::string& shaderInterfacePath;
        const engine::string& shaderCorePath;
        const engine::string& shaderLoadInterfaceTemplateHeader;
        const engine::string& shaderLoadInterfaceTemplateSource;
        const engine::string& shaderPipelineInterfaceTemplateHeader;
        const engine::string& shaderPipelineInterfaceTemplateSource;
        engine::string singleFile = "";
        bool forceCompileAll = false;
    };

    class ShaderLocator
    {
    public:
        ShaderLocator(const ShaderLocatorParameters& parameters);

        engine::vector<ShaderPipelineConfiguration>& pipelines()
        {
            return m_pipelineVector;
        }

        const engine::string& shaderRootPath() const { return m_shaderRootPath; }
        const engine::string& shaderBinaryPathDX12() const { return m_shaderBinaryPathDX12; }
        const engine::string& shaderBinaryPathVulkan() const { return m_shaderBinaryPathVulkan; }
        const engine::string& shaderInterfacePath() const { return m_shaderInterfacePath; }
        const engine::string& shaderCorePath() const { return m_shaderCorePath; }
        const engine::string& shaderLoadInterfaceTemplateHeader() const { return m_shaderLoadInterfaceTemplateHeader; }
        const engine::string& shaderLoadInterfaceTemplateSource() const { return m_shaderLoadInterfaceTemplateSource; }
        const engine::string& shaderPipelineInterfaceTemplateHeader() const { return m_shaderPipelineInterfaceTemplateHeader; }
        const engine::string& shaderPipelineInterfaceTemplateSource() const { return m_shaderPipelineInterfaceTemplateSource; }
        const engine::string& commonPath() const { return m_commonPath; }

    private:
        engine::string m_shaderRootPath;
        engine::string m_shaderBinaryPathDX12;
        engine::string m_shaderBinaryPathVulkan;
        engine::string m_shaderInterfacePath;
        engine::string m_shaderCorePath;
        engine::string m_shaderLoadInterfaceTemplateHeader;
        engine::string m_shaderLoadInterfaceTemplateSource;
        engine::string m_shaderPipelineInterfaceTemplateHeader;
        engine::string m_shaderPipelineInterfaceTemplateSource;
        engine::string m_commonPath;
        void locatePipelines();
        engine::unordered_map<engine::string, ShaderPipelineConfiguration> m_pipelines;
        engine::vector<ShaderPipelineConfiguration> m_pipelineVector;

        void removeUnchangedPipelines();
        void forceFullRebuild();
    };
}
