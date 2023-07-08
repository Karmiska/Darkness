#include "ShaderLocator.h"
#include "platform/Directory.h"
#include "tools/PathTools.h"
#include "ShaderPathTools.h"
#include "platform/File.h"
#include "ShaderCompilerCommon.h"
#include <functional>
#include <algorithm>
#include <execution>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <atomic>

namespace fs = std::filesystem;

#define MULTITHREADED_REMOVEUNCHANGED

namespace shadercompiler
{
    engine::vector<engine::string> collectIncludes(const engine::string& file)
    {
        std::fstream filestream;
        filestream.open(file);
        ASSERT(filestream.is_open(), "Failed to open preprocessed file!");

        filestream.seekg(0, std::ios::end);
        auto fileSize = static_cast<size_t>(filestream.tellg());
        filestream.seekg(0, std::ios::beg);

        engine::vector<char> data(fileSize + 1);
        filestream.read(data.data(), fileSize);
        filestream.close();
        data[fileSize] = '\0';

        auto lines = readLines(data);
        engine::vector<engine::string> result;
        for (auto&& line : lines)
        {
            if (line.find("#include") != engine::string::npos)
            {
                auto includeStart = line.find("\"") + 1;
                auto includeStop = line.find("\"", includeStart);
                auto include = line.substr(includeStart, includeStop - includeStart);

                // fix include path
                auto absolutePath = engine::pathClean(engine::pathJoin(engine::pathExtractFolder(file), include));

                result.emplace_back(absolutePath);
                auto includeLines = collectIncludes(absolutePath);
                for (auto&& l : includeLines)
                    result.emplace_back(l);
            }
        }
        return result;
    }

    int ShaderPipelineStage::nonBindlessSetCount() const
    {
        return
            ((items[0].resources->textureSRV().size() > 0 ? 1 : 0) ||
                (items[0].resources->textureUAV().size() > 0 ? 1 : 0) ||
                (items[0].resources->bufferSRV().size() > 0 ? 1 : 0) ||
                (items[0].resources->bufferUAV().size() > 0 ? 1 : 0) ||
                (items[0].resources->accelerationStructures().size() > 0 ? 1 : 0) ||
                (items[0].resources->samplers().size() > 0 ? 1 : 0) ||
                (items[0].resources->rootConstants().size() > 0 ? 1 : 0));
    };
    
    int ShaderPipelineStage::setCount() const
    {
        return
            std::max(
                static_cast<int>(nonBindlessSetCount()) +
                static_cast<int>(items[0].resources->bindlessTextureSRV().size()) +
                static_cast<int>(items[0].resources->bindlessTextureUAV().size()) +
                static_cast<int>(items[0].resources->bindlessBufferSRV().size()) +
                static_cast<int>(items[0].resources->bindlessBufferUAV().size()),
                1);

    };

    engine::string getCommonParent(const engine::string& path1, const engine::string& path2)
    {
        engine::string ret = path2;

        while (path1.find(ret) == engine::string::npos)
            ret.erase(ret.end() - 1);

        if (ret.empty())
            return ret;

        while (ret[ret.length() - 1] == '\\' || !ret[ret.length() - 1] == '/')
            ret.erase(ret.end() - 1);

        return ret;
    }

    ShaderLocator::ShaderLocator(const ShaderLocatorParameters& parameters)
        : m_shaderRootPath{ parameters.shaderRootPath }
        , m_shaderBinaryPathDX12{ parameters.shaderBinaryPathDX12 }
        , m_shaderBinaryPathVulkan{ parameters.shaderBinaryPathVulkan }
        , m_shaderInterfacePath{ parameters.shaderInterfacePath }
        , m_shaderCorePath{ parameters.shaderCorePath }
        , m_shaderLoadInterfaceTemplateHeader{ parameters.shaderLoadInterfaceTemplateHeader }
        , m_shaderLoadInterfaceTemplateSource{ parameters.shaderLoadInterfaceTemplateSource }
        , m_shaderPipelineInterfaceTemplateHeader{ parameters.shaderPipelineInterfaceTemplateHeader }
        , m_shaderPipelineInterfaceTemplateSource{ parameters.shaderPipelineInterfaceTemplateSource }
        , m_commonPath{ getCommonParent(m_shaderRootPath, m_shaderBinaryPathDX12) }
    {
        auto lookForOtherStagesInPipeline = [](const engine::string& sourceFile)->engine::vector<ShaderPipelineStage>
        {
            engine::vector<ShaderPipelineStage> result;

            auto pipelineName = pipelineNameFromFilename(sourceFile);
            auto dir = engine::pathExtractFolder(sourceFile);
            engine::Directory pipelineDir(dir);
            for (auto&& file : pipelineDir.files())
            {
                auto tempPipelineName = pipelineNameFromFilename(file);
                if (tempPipelineName == pipelineName)
                {
                    if (file.ends_with(".hlsl"))
                    {
                        auto stage = stageName(file);
                        result.emplace_back(ShaderPipelineStage{ stage, engine::pathJoin(engine::pathExtractFolder(sourceFile), file) });
                        result.back().buildCodeInterface = true;
                        result.back().buildShaderBinary = true;
                    }
                }
            }

            // move vertex to first stage
            for (int i = 0; i < result.size(); ++i)
            {
                if (result[i].name == "Vertex")
                {
                    auto temp = result[i];
                    result.erase(result.begin() + i);
                    result.insert(result.begin(), temp);
                }
            }

            return result;
        };

        if (parameters.singleFile.empty())
        {
            locatePipelines();
            if (!parameters.forceCompileAll)
                removeUnchangedPipelines();
            else
                forceFullRebuild();

            // add other stages to changed pipelines
            for (auto&& pipeline : m_pipelineVector)
            {
                pipeline.stages = lookForOtherStagesInPipeline(pipeline.stages[0].filename);
            }
        }
        else
        {
            auto allStages = lookForOtherStagesInPipeline(parameters.singleFile);

            ShaderPipelineConfiguration conf;
            conf.path = engine::pathExtractFolder(parameters.singleFile);
            conf.pipelineName = pipelineNameFromFilename(parameters.singleFile);
            conf.buildCodeInterface = false;
            conf.stages = allStages;
            m_pipelineVector.emplace_back(conf);
        }
    }

    using OnAddFile = std::function<void(const engine::string&)>;

    void walkDirectory(const engine::string& path, OnAddFile onAddFile = {})
    {
        engine::Directory dir(path);
        for (auto&& d : dir.directories())
            walkDirectory(engine::pathJoin(path, d), onAddFile);

        for (auto&& f : dir.files())
            onAddFile(engine::pathJoin(path, f));
    };

    void ShaderLocator::locatePipelines()
    {
        walkDirectory(m_shaderRootPath, [&](const engine::string& filePath)
        {
            if (filePath.substr(filePath.length() - 5, 5) != ".hlsl")
                return;
            auto pipelineName = pipelineNameFromFilename(filePath);
            auto dir = engine::pathExtractFolder(filePath);
            auto pipelineHash = sha1(dir + pipelineName);

            auto exists = m_pipelines.find(pipelineHash);
            if (exists == m_pipelines.end())
            {
                ShaderPipelineConfiguration conf;
                conf.path = dir;
                conf.pipelineName = pipelineName;
                m_pipelines[pipelineHash] = conf;
                exists = m_pipelines.find(pipelineHash);
            }

            auto stage = stageName(filePath);
            (*exists).second.stages.emplace_back(ShaderPipelineStage{ stage, filePath });
        });

        for (auto&& pipeline : m_pipelines)
            m_pipelineVector.emplace_back(pipeline.second);

        m_pipelines.clear();

        for (auto&& pipe : m_pipelineVector)
        {
            for (int i = 0; i < pipe.stages.size(); ++i)
            {
                if (pipe.stages[i].name == "Vertex")
                {
                    auto temp = pipe.stages[i];
                    pipe.stages.erase(pipe.stages.begin() + i);
                    pipe.stages.insert(pipe.stages.begin(), temp);
                }
            }
        }
    }

    void ShaderLocator::removeUnchangedPipelines()
    {
        // change rules
        //
        // 1. if shader load templates are older than any shader interface. 
        //      - rebuild shader load c++ interfaces
        // 2. if shader pipeline templates are older than any c++ interfaces
        //      - rebuild pipeline c++ templates
        // 2. if HLSL source or any of it's includes is older than the shader binary. 
        //      - rebuild shader binary
        //      - rebuild c++ interfaces
        // 

        auto shaderLoadTemplateHeaderTimestamp = fs::last_write_time(shaderLoadInterfaceTemplateHeader());
        auto shaderLoadTemplateSourceTimestamp = fs::last_write_time(shaderLoadInterfaceTemplateSource());
        auto shaderPipelineTemplateHeaderTimestamp = fs::last_write_time(shaderPipelineInterfaceTemplateHeader());
        auto shaderPipelineTemplateSourceTimestamp = fs::last_write_time(shaderPipelineInterfaceTemplateSource());
        std::atomic_bool anyInterfaceChange = false;

#ifdef MULTITHREADED_REMOVEUNCHANGED
        std::for_each(
            std::execution::par_unseq,
            m_pipelineVector.begin(),
            m_pipelineVector.end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : m_pipelineVector)
#endif
        {
#ifdef MULTITHREADED_REMOVEUNCHANGED
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                // get the most recent timestamp for HLSL source and it's includes
                stage.sourceTimestamp = fs::last_write_time(stage.filename);
                auto includes = collectIncludes(stage.filename);
                for (auto&& include : includes)
                {
                    fs::file_time_type includeTime = fs::last_write_time(include);
                    if (includeTime > stage.sourceTimestamp)
                        stage.sourceTimestamp = includeTime;
                }

                // get the shader binary timestamp
                auto binFolderRelative = stage.filename.substr(shaderRootPath().length(), stage.filename.length() - shaderRootPath().length());
                        
                // check for shader binary timestamps
                {
                    auto binaryTarget = engine::pathJoin(shaderBinaryPathDX12(), binFolderRelative);
                    auto folder = engine::pathExtractFolder(binaryTarget);
                    auto permFile = engine::pathExtractFilename(stage.filename);
                    auto outputpath = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "cso");

                    auto binaryPath = outputpath;
                    bool binaryExists = engine::fileExists(binaryPath);
                    if (!binaryExists)
                    {
                        // maybe it has a permutation target. test the first one
                        binaryPath = engine::pathJoin(
                            engine::pathExtractFolder(binaryPath),
                            engine::pathExtractFilenameWithoutExtension(binaryPath) + "_" + "000" + ".cso");
                        binaryExists = engine::fileExists(binaryPath);
                    }

                    if (binaryExists)
                    {
                        stage.binaryTimestamp = fs::last_write_time(binaryPath);
                    }

                    stage.buildShaderBinary = stage.binaryTimestamp < stage.sourceTimestamp;
                }

                // check for shader load interface timestamps
                {
                    stage.buildCodeInterface = false;
                    auto binaryTarget = engine::pathJoin(shaderInterfacePath(), binFolderRelative);
                    auto hppFile = engine::pathReplaceExtension(binaryTarget, "h");

                    if (engine::fileExists(hppFile))
                    {
                        stage.codeInterfacesTimestamp = fs::last_write_time(hppFile);
                    }

                    auto interfaceChange =
                        stage.codeInterfacesTimestamp < shaderLoadTemplateHeaderTimestamp ||
                        stage.codeInterfacesTimestamp < shaderLoadTemplateSourceTimestamp ||
                        stage.codeInterfacesTimestamp < shaderPipelineTemplateHeaderTimestamp ||
                        stage.codeInterfacesTimestamp < shaderPipelineTemplateSourceTimestamp;

                    if (interfaceChange)
                    {
                        anyInterfaceChange = true;
                    }
                }
            }
#ifdef MULTITHREADED_REMOVEUNCHANGED
            );
#endif
            pipeline.buildCodeInterface = false;
        }
#ifdef MULTITHREADED_REMOVEUNCHANGED
        );
#endif

        // if no interfaces had been updated (ie. templates changed)
        // we can actually reduce all the pipelines to just those that have shader source changes
        if (!anyInterfaceChange)
        {
            for (auto it = m_pipelineVector.begin(); it != m_pipelineVector.end();)
            {
                for (auto st = (*it).stages.begin(); st != (*it).stages.end();)
                {
                    auto& stage = *st;

                    if (!stage.buildShaderBinary)
                    {
                        st = (*it).stages.erase(st);
                    }
                    else
                    {
                        stage.buildCodeInterface = true;
                        ++st;
                    }
                }
                if ((*it).stages.size() == 0)
                    it = m_pipelineVector.erase(it);
                else
                    ++it;
            }
        }
        else
        {
#ifdef MULTITHREADED_REMOVEUNCHANGED
            std::for_each(
                std::execution::par_unseq,
                m_pipelineVector.begin(),
                m_pipelineVector.end(),
                [&](auto&& pipeline)
#else
            for (auto&& pipeline : m_pipelineVector)
#endif
            {
#ifdef MULTITHREADED_REMOVEUNCHANGED
                std::for_each(
                    std::execution::par_unseq,
                    pipeline.stages.begin(),
                    pipeline.stages.end(),
                    [&](auto&& stage)
#else
                for (auto&& stage : pipeline.stages)
#endif
                {
                    stage.buildCodeInterface = true;
                }
#ifdef MULTITHREADED_REMOVEUNCHANGED
                );
                pipeline.buildCodeInterface = true;
#endif
            }
#ifdef MULTITHREADED_REMOVEUNCHANGED
            );
#endif
        }
    }

    void ShaderLocator::forceFullRebuild()
    {

#ifdef MULTITHREADED_REMOVEUNCHANGED
        std::for_each(
            std::execution::par_unseq,
            m_pipelineVector.begin(),
            m_pipelineVector.end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : m_pipelineVector)
#endif
        {
#ifdef MULTITHREADED_REMOVEUNCHANGED
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                stage.buildShaderBinary = true;
                stage.buildCodeInterface = true;
            }
#ifdef MULTITHREADED_REMOVEUNCHANGED
            );
#endif
            pipeline.buildCodeInterface = true;
        }
#ifdef MULTITHREADED_REMOVEUNCHANGED
        );
#endif
    }
}
