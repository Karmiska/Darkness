#include "CompileTask.h"
#include "Preprocessor.h"
#include "ResourceMap.h"
#include "Helpers.h"
#include "TemplateProcessor.h"
#include "tools/Debug.h"
#include "tools/PathTools.h"
#include "ShaderLocator.h"
#include "platform/Directory.h"
#include "platform/File.h"
#include "tools/Process.h"
#include "platform/Environment.h"
#include <algorithm>
#include <execution>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

#define MULTITHREADED_PREPROCESS
#define MULTITHREADED_WRITEROOTSIGNATURES
#define MULTITHREADED_UPDATERESOURCEREGISTERS
#define MULTITHREADED_SHADERCOMPILE
#define MULTITHREADED_PATCHVULKANREGISTERS
#define MULTITHREADED_CODEINTERFACES

namespace shadercompiler
{
    CompileTask::CompileTask(ShaderLocator& locator, bool optimization, LogLevel logLevel)
        : m_logLevel{ logLevel }
    {
        m_shaderProfiles["Compute"] = "cs_6_3";
        m_shaderProfiles["Domain"] = "ds_6_3";
        m_shaderProfiles["Geometry"] = "gs_6_3";
        m_shaderProfiles["Hull"] = "hs_6_3";
        m_shaderProfiles["Pixel"] = "ps_6_3";
        m_shaderProfiles["Vertex"] = "vs_6_3";
        m_shaderProfiles["Raygeneration"] = "lib_6_3";
        m_shaderProfiles["Intersection"] = "lib_6_3";
        m_shaderProfiles["Miss"] = "lib_6_3";
        m_shaderProfiles["AnyHit"] = "lib_6_3";
        m_shaderProfiles["ClosestHit"] = "lib_6_3";
        m_shaderProfiles["Amplification"] = "as_6_5";
        m_shaderProfiles["Mesh"] = "ms_6_5";

        if(doLog(LogLevel::Progress)) LOG_PURE("Preprocess and get resources (2/8)");
        preprocessAndGetResources(locator);

        if(doLog(LogLevel::Progress)) LOG_PURE("Write root signatures (3/8)");
        writeRootSignatures(locator);

        if(doLog(LogLevel::Progress)) LOG_PURE("Update resource registers (4/8)");
        updateResourceRegisters(locator);

        if(doLog(LogLevel::Progress)) LOG_PURE("Compile DX12 shaders (5/8)");
        compile(locator, optimization);

        if(doLog(LogLevel::Progress)) LOG_PURE("Patch Vulkan shader registers (6/8)");
        patchVulkanRegisters(locator);

        if(doLog(LogLevel::Progress)) LOG_PURE("Compile Vulkan shaders (7/8)");
        compile(locator, optimization, GraphicsApi::Vulkan);

        if(doLog(LogLevel::Progress)) LOG_PURE("Create c++ interfaces (8/8)");
        createCodeInterfaces(locator);
    }

    bool CompileTask::doLog(LogLevel level) const
    {
        if (m_logLevel == LogLevel::None)
            return false;

        return static_cast<int>(level) <= static_cast<int>(m_logLevel);
    }

    std::vector<std::vector<PermutationItem>> cartesianProduct(engine::vector<engine::vector<PermutationItem>>& lists)
    {
        if (lists.size() == 0)
            return {};

        std::vector<std::vector<PermutationItem>> result;
        if (std::find_if(std::begin(lists), std::end(lists),
            [](auto e) -> bool { return e.size() == 0; }) != std::end(lists)) {
            return result;
        }
        for (auto& e : lists[0]) {
            result.push_back({ e });
        }
        for (size_t i = 1; i < lists.size(); ++i) {
            std::vector<std::vector<PermutationItem>> temp;
            for (auto& e : result) {
                for (auto f : lists[i]) {
                    auto e_tmp = e;
                    e_tmp.push_back(f);
                    temp.push_back(e_tmp);
                }
            }
            result = temp;
        }
        return result;
    };

    void CompileTask::preprocessAndGetResources(ShaderLocator& locator)
    {
#ifdef MULTITHREADED_PREPROCESS
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
            auto optionPermute = [](const engine::vector<Condition>& options)->engine::vector<engine::vector<PermutationItem>>
            {
                engine::vector<engine::vector<PermutationItem>> result;
                uint64_t count = static_cast<uint64_t>(1u) << static_cast<uint64_t>(options.size());
                uint64_t currentValue = static_cast<uint64_t>(0u);
                for (int i = 0; i < count; ++i)
                {
                    engine::vector<PermutationItem> res;
                    for (uint64_t a = 0; a < options.size(); ++a)
                        res.emplace_back(PermutationItem{
                        "option",
                        options[a].value,
                        ((currentValue & (static_cast<uint64_t>(1) << a)) == (static_cast<uint64_t>(1) << a)) ? "true" : "false",
                        options[a].define });
                    ++currentValue;
                    result.emplace_back(res);
                }
                return result;
            };

            auto enumPermute = [](const engine::unordered_map<engine::string, engine::vector<Condition>>& enums)->engine::vector<engine::vector<PermutationItem>>
            {
                engine::vector<engine::vector<PermutationItem>> result;
                for (auto item : enums)
                {
                    engine::vector<PermutationItem> res;
                    for (auto cond : item.second)
                    {
                        auto varName = item.first;
                        std::transform(varName.begin(), varName.end(), varName.begin(), ::tolower);

                        auto val = item.first;
                        std::transform(val.begin(), val.end(), val.begin(), ::toupper);
                        std::transform(val.begin() + 1, val.end(), val.begin() + 1, ::tolower);
                        val += "::" + cond.value;

                        res.emplace_back(PermutationItem{ "enum", varName, val, cond.define });
                    }
                    result.emplace_back(res);
                }
                if (result.size() == 0)
                {
                    engine::vector<PermutationItem> temp;
                    result.emplace_back(temp);
                }

                auto res = cartesianProduct(result);
                if (res.size() == 0)
                    res.emplace_back(engine::vector<PermutationItem>());
                return res;
            };

            auto permute = [&](const engine::vector<Condition>& options, const engine::unordered_map<engine::string, engine::vector<Condition>>& enums)->engine::vector<Permutation>
            {
                auto op = optionPermute(options);
                auto en = enumPermute(enums);

                engine::vector<Permutation> result;
                int permutationId = 0;
                for (auto&& o : op)
                {
                    for (auto&& e : en)
                    {
                        engine::vector<PermutationItem> temp;
                        temp.reserve(temp.size() + distance(o.begin(), o.end()));
                        temp.insert(temp.end(), o.begin(), o.end());
                        temp.reserve(temp.size() + distance(e.begin(), e.end()));
                        temp.insert(temp.end(), e.begin(), e.end());
                        if (temp.size() > 0)
                        {
                            Permutation perm;
                            perm.list = temp;

                            std::stringstream ss;
                            ss << std::setw(3) << std::setfill('0') << permutationId;
                            perm.id = ss.str();

                            engine::vector<engine::string> flags;
                            for (auto&& _perm : perm.list)
                            {
                                if ((_perm.type == "option") && (_perm.value == "true"))
                                    flags.emplace_back(_perm.flag);
                                if (_perm.type == "enum")
                                    flags.emplace_back(_perm.flag);
                            }
                            perm.defines = flags;
                            result.emplace_back(perm);
                            ++permutationId;
                        }
                    }
                }
                return result;
            };

#ifdef MULTITHREADED_PREPROCESS
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                shadercompiler::Preprocessor preprocessor(stage.filename);
                stage.enums = preprocessor.enums();
                stage.options = preprocessor.options();
                stage.permutations = permute(preprocessor.options(), preprocessor.enums());

                auto preprocessedFilename = [&](const engine::string& path)->engine::string
                {
                    auto noext = engine::pathExtractFilenameWithoutExtension(path);
                    return engine::pathJoin(engine::pathExtractFolder(path), noext + ".preprocessed");
                };

                if (stage.permutations.size() == 0)
                {
                    ShaderPipelineStageItem item;
                    item.permutationPath = stage.filename;
                    item.sourcePath = stage.filename;
                    item.recursiveContent = preprocessor.process(
                        stage.filename, 
                        {}, 
                        {}, 
                        preprocessedFilename(stage.filename),
                        stage.shaderIncludeDepencyPaths);
                    item.resources = engine::make_shared<shadercompiler::ResourceMap>(item.recursiveContent);
                    {
                        stage.items.emplace_back(item);
                    }
                }
                else
                {
                    stage.itemMutex = engine::make_shared<std::mutex>();
#ifdef MULTITHREADED_PREPROCESS
                    std::for_each(
                        std::execution::par_unseq,
                        stage.permutations.begin(),
                        stage.permutations.end(),
                        [&](auto&& permutation)
#else
                    for (auto&& permutation : stage.permutations)
#endif
                    {
                        ShaderPipelineStageItem item;

                        item.permutationPath = engine::pathJoin(
                            engine::pathExtractFolder(stage.filename),
                            engine::pathExtractFilenameWithoutExtension(stage.filename) + "_" + permutation.id + ".cso");
                        item.sourcePath = stage.filename;
                        engine::vector<engine::string> shaderIncludeDepencyPaths;
                        item.recursiveContent = preprocessor.process(
                            stage.filename, 
                            {}, 
                            permutation.defines, 
                            preprocessedFilename(item.permutationPath),
                            shaderIncludeDepencyPaths);
                        item.resources = engine::make_shared<shadercompiler::ResourceMap>(item.recursiveContent);

                        {
                            std::lock_guard<std::mutex> lock(*stage.itemMutex);
                            stage.items.emplace_back(item);
                            stage.shaderIncludeDepencyPaths = shaderIncludeDepencyPaths;
                        }
                    }
#ifdef MULTITHREADED_PREPROCESS
                    );
#endif
                }
            } // for end
#ifdef MULTITHREADED_PREPROCESS
            );
#endif

        }
#ifdef MULTITHREADED_PREPROCESS
        );
#endif
    }

    void CompileTask::writeRootSignatures(ShaderLocator& locator) const
    {
#ifdef MULTITHREADED_WRITEROOTSIGNATURES
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
            // update set values
            auto setCount = 0;
            for (auto&& stage : pipeline.stages)
            {
                stage.setStart = setCount;
                setCount += stage.setCount();
            }

#ifdef MULTITHREADED_WRITEROOTSIGNATURES
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                if (stage.buildShaderBinary)
                {
                    // get target binary file path
                    auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
                    auto binaryTarget = engine::pathJoin(locator.shaderBinaryPathDX12(), binFolderRelative);
                    auto binaryTargetFolder = engine::pathExtractFolder(binaryTarget);

                    // make sure the directory is there
                    engine::Directory targetFolder(binaryTargetFolder);
                    if (!targetFolder.exists())
                        targetFolder.create();

                    // create root signature file path
                    auto rootSignaturePath = engine::pathReplaceExtension(binaryTarget, "rs");

                    // write root signature
                    std::ofstream rootSignature;
                    rootSignature.open(rootSignaturePath+"temporary", std::ios::out);
                    writeRootSignature(rootSignature, stage);
                    rootSignature.close();
                    if (compareAndReplace(rootSignaturePath + "temporary", rootSignaturePath))
                    {
                        if (m_logLevel == shadercompiler::LogLevel::Recompile)
                            LOG_PURE("Updated RootSignature for %s", engine::pathExtractFilenameWithoutExtension(binaryTarget).c_str());
                    }

                    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                    compileRootSignature(rootSignaturePath, engine::pathReplaceExtension(rootSignaturePath, "rso"));
                }
            }
#ifdef MULTITHREADED_WRITEROOTSIGNATURES
            );
#endif
        }
#ifdef MULTITHREADED_WRITEROOTSIGNATURES
        );
#endif
    }

    void CompileTask::compileRootSignature(const engine::string& rs, const engine::string& rso) const
    {
        auto dxcPath = engine::pathClean("C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0/x86/dxc.exe");
        {
            // in it's own scope so we wait for the execution to finish
            engine::string arguments = " /Vd /T rootsig_1_1 " + rs + " /Fo " + rso;

            bool done = false;
            int error = 15;
            while (!done)
            {
                done = true;
                engine::Process process(dxcPath, arguments, engine::pathExtractFolder(engine::getExecutableDirectory()), [&](const engine::string& msg)
                {
                    if (msg.find("The process cannot access the file because it is being used by another process") != engine::string::npos)
                    {
                        done = false;
                    }
                    else
                    {
                        done = false;
                        --error;
                        if (error == 0)
                            if (doLog(LogLevel::Error)) LOG("Preprocessors message: %s", msg.c_str());
                    }
                });

                if (error == 0)
                    break;

                if (!done)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }

    void CompileTask::writeRootSignature(std::ofstream& stream, const ShaderPipelineStage& stage) const
    {
        auto isDXRShader = [](const ShaderPipelineStage& stage)
        {
            return
                stage.name == "Raygeneration" ||
                stage.name == "Intersection" ||
                stage.name == "Miss" ||
                stage.name == "AnyHit" ||
                stage.name == "ClosestHit";
        };

        engine::string visibility;
        if (stage.name == "Vertex") visibility = ", visibility=SHADER_VISIBILITY_VERTEX";
        else if (stage.name == "Pixel") visibility = ", visibility=SHADER_VISIBILITY_PIXEL";
        else if (stage.name == "Compute") visibility = ", visibility=SHADER_VISIBILITY_ALL";
        else if (stage.name == "Domain") visibility = ", visibility=SHADER_VISIBILITY_DOMAIN";
        else if (stage.name == "Geometry") visibility = ", visibility=SHADER_VISIBILITY_GEOMETRY";
        else if (stage.name == "Hull") visibility = ", visibility=SHADER_VISIBILITY_HULL";
        else if (stage.name == "Amplification") visibility = ", visibility=SHADER_VISIBILITY_AMPLIFICATION";
        else if (stage.name == "Mesh") visibility = ", visibility=SHADER_VISIBILITY_MESH";
        else visibility = ", visibility=SHADER_VISIBILITY_ALL";

        if (isDXRShader(stage))
        {
            engine::string msg = "#define main \"RootFlags(LOCAL_ROOT_SIGNATURE)";
            stream.write(msg.c_str(), msg.length());
        }
        else
        {
            engine::string msg = "#define main \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)";
            stream.write(msg.c_str(), msg.length());
        }

        engine::string lineContinue = ", \" \\\n";
        engine::string lineEnd = "\"";

        int b = 0;
        int t = 0;
        int u = 0;
        int space = stage.setStart;
        int bindlessSpace = space + stage.nonBindlessSetCount();

        for (auto&& rc : stage.items[0].resources->rootConstants())
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"RootConstants(num32BitConstants=1, b" + std::to_string(b) + ")";
            stream.write(msg.c_str(), msg.length());
            ++b;
        }

        if(stage.items[0].resources->samplers().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(Sampler(s0, space = " + std::to_string(space) + ", numDescriptors = "+std::to_string(stage.items[0].resources->samplers().size())+"))";
            stream.write(msg.c_str(), msg.length());
        }

        if (stage.items[0].resources->textureSRV().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(t"+ std::to_string(t)+", space = " + std::to_string(space) + ", numDescriptors = "+ std::to_string(stage.items[0].resources->textureSRV().size())+", flags = DESCRIPTORS_VOLATILE)"+visibility+")";
            stream.write(msg.c_str(), msg.length());
            t += stage.items[0].resources->textureSRV().size();
        }
        if (stage.items[0].resources->textureUAV().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(UAV(u" + std::to_string(u) + ", space = " + std::to_string(space) + ", numDescriptors = " + std::to_string(stage.items[0].resources->textureUAV().size()) + ", flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            u += stage.items[0].resources->textureUAV().size();
        }

        if (stage.items[0].resources->bufferSRV().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(t" + std::to_string(t) + ", space = " + std::to_string(space) + ", numDescriptors = " + std::to_string(stage.items[0].resources->bufferSRV().size()) + ", flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            t += stage.items[0].resources->bufferSRV().size();
        }
        if (stage.items[0].resources->bufferUAV().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(UAV(u" + std::to_string(u) + ", space = " + std::to_string(space) + ", numDescriptors = " + std::to_string(stage.items[0].resources->bufferUAV().size()) + ", flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            u += stage.items[0].resources->bufferUAV().size();
        }

        if (stage.items[0].resources->accelerationStructures().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(t" + std::to_string(t) + ", space = "+ std::to_string(space) +", numDescriptors = " + std::to_string(stage.items[0].resources->accelerationStructures().size()) + ", flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            t += stage.items[0].resources->accelerationStructures().size();
        }

        for (auto&& res : stage.items[0].resources->bindlessTextureSRV())
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(t0, space = "+ std::to_string(bindlessSpace) +", numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)"+visibility+")";
            stream.write(msg.c_str(), msg.length());
            ++bindlessSpace;
        }
        for (auto&& res : stage.items[0].resources->bindlessBufferSRV())
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(t0, space = " + std::to_string(bindlessSpace) + ", numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            ++bindlessSpace;
        }
        for (auto&& res : stage.items[0].resources->bindlessTextureUAV())
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(u0, space = " + std::to_string(bindlessSpace) + ", numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            ++bindlessSpace;
        }
        for (auto&& res : stage.items[0].resources->bindlessBufferUAV())
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(SRV(u0, space = " + std::to_string(bindlessSpace) + ", numDescriptors = unbounded, flags = DESCRIPTORS_VOLATILE)" + visibility + ")";
            stream.write(msg.c_str(), msg.length());
            ++bindlessSpace;
        }

        if (stage.items[0].resources->constantBuffers().size() > 0)
        {
            stream.write(lineContinue.c_str(), lineContinue.length());

            auto msg = "              \"DescriptorTable(CBV(b"+ std::to_string(b)+", space = " + std::to_string(space) + ", numDescriptors = "+std::to_string(stage.items[0].resources->constantBuffers().size())+")"+visibility+")";
            stream.write(msg.c_str(), msg.length());
            b += stage.items[0].resources->constantBuffers().size();
        }

        stream.write(lineEnd.c_str(), lineEnd.length());
    }

    void CompileTask::updateResourceRegisters(ShaderLocator& locator)
    {
#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
                auto binaryTarget = engine::pathJoin(locator.shaderBinaryPathDX12(), binFolderRelative);
                auto binaryTargetFolder = engine::pathExtractFolder(binaryTarget);

                // make sure the directory is there
                engine::Directory targetFolder(binaryTargetFolder);
                if (!targetFolder.exists())
                    targetFolder.create();

#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
                std::for_each(
                    std::execution::par_unseq,
                    stage.items.begin(),
                    stage.items.end(),
                    [&](auto&& item)
#else
                for (auto&& item : stage.items)
#endif
                {
                    engine::vector<engine::string> newData(item.recursiveContent);

                    int b = 0;
                    int t = 0;
                    int u = 0;
                    int s = 0;
                    int space = stage.setStart;
                    int bindlessSpace = space + stage.nonBindlessSetCount();


                    int vulkanBindingIndex = 0;

                    for (auto&& res : item.resources->rootConstants())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(b" + std::to_string(b) + ", space" + std::to_string(space) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        }
                        ++b;
                    }

                    for (auto&& res : item.resources->samplers())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(s" + std::to_string(s) + ", space" + std::to_string(space) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        }
                        ++s;
                    }

                    for (auto&& res : item.resources->textureSRV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(t" + std::to_string(t) + ", space" + std::to_string(space) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        }
                        ++t;
                    }

                    for (auto&& res : item.resources->textureUAV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(u" + std::to_string(u) + ", space" + std::to_string(space) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        }
                        ++u;
                    }

                    for (auto&& res : item.resources->bufferSRV())
                    {
                        if (res.structured)
                        {
                            ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                            auto& line = newData[res.lineNumber];
                            if (line.find("register") == engine::string::npos)
                            {
                                line.insert(line.rfind(";"), " : register(t" + std::to_string(t) + ", space" + std::to_string(space) + ")");
                                item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                            }
                            ++t;
                        }
                    }

                    for (auto&& res : item.resources->bufferSRV())
                    {
                        if (!res.structured)
                        {
                            ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                            auto& line = newData[res.lineNumber];
                            if (line.find("register") == engine::string::npos)
                            {
                                line.insert(line.rfind(";"), " : register(t" + std::to_string(t) + ", space" + std::to_string(space) + ")");
                                item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                            }
                            ++t;
                        }
                    }

                    for (auto&& res : item.resources->bufferUAV())
                    {
                        if (res.structured)
                        {
                            ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                            auto& line = newData[res.lineNumber];
                            if (line.find("register") == engine::string::npos)
                            {
                                line.insert(line.rfind(";"), " : register(u" + std::to_string(u) + ", space" + std::to_string(space) + ")");
                                item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                            }
                            ++u;
                        }
                    }

                    for (auto&& res : item.resources->bufferUAV())
                    {
                        if (!res.structured)
                        {
                            ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                            auto& line = newData[res.lineNumber];
                            if (line.find("register") == engine::string::npos)
                            {
                                line.insert(line.rfind(";"), " : register(u" + std::to_string(u) + ", space" + std::to_string(space) + ")");
                                item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                            }
                            ++u;
                        }
                    }

                    for (auto&& res : item.resources->accelerationStructures())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(t" + std::to_string(u) + ", space" + std::to_string(space) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        }
                        ++t;
                    }

                    for (auto&& res : item.resources->bindlessTextureSRV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(t0, space" + std::to_string(bindlessSpace) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, 0, bindlessSpace });
                        }
                        ++bindlessSpace;
                    }

                    for (auto&& res : item.resources->bindlessBufferSRV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(t0, space" + std::to_string(bindlessSpace) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, 0, bindlessSpace });
                        }
                        ++bindlessSpace;
                    }

                    for (auto&& res : item.resources->bindlessTextureUAV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(u0, space" + std::to_string(bindlessSpace) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, 0, bindlessSpace });
                        }
                        ++bindlessSpace;
                    }

                    for (auto&& res : item.resources->bindlessBufferUAV())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.find("register") == engine::string::npos)
                        {
                            line.insert(line.rfind(";"), " : register(u0, space" + std::to_string(bindlessSpace) + ")");
                            item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, 0, bindlessSpace });
                        }
                        ++bindlessSpace;
                    }

                    for (auto&& res : item.resources->constantBuffers())
                    {
                        ASSERT(res.lineNumber < newData.size(), "Resource line number out of range");
                        auto& line = newData[res.lineNumber];
                        if (line.rfind(";") != engine::string::npos)
                            line.insert(line.rfind(";"), " : register(b" + std::to_string(b) + ", space" + std::to_string(space) + ")");
                        else
                            line.insert(line.length() - 1, " : register(b" + std::to_string(b) + ", space" + std::to_string(space) + ")");
                        item.vulkanBindings.emplace_back(ShaderPipelineStageItem::VulkanBinding{ res.lineNumber, vulkanBindingIndex, space }); ++vulkanBindingIndex;
                        ++b;
                    }

                    // create static resources file path
                    engine::string path;
                    if (stage.permutations.size() == 0) 
                        path = engine::pathReplaceExtension(binaryTarget, "static_resources");
                    else
                    {
                        auto folder = engine::pathExtractFolder(binaryTarget);
                        auto permFile = engine::pathExtractFilename(item.permutationPath);
                        path = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "static_resources");
                    }

                    if (engine::fileExists(path + "temporary"))
                        engine::fileDelete(path + "temporary");

                    std::ofstream staticResources;
                    staticResources.open(path + "temporary", std::ios::out);
                    ASSERT(staticResources.is_open(), "Cant output static resource content");
                    for (auto&& line : newData)
                    {
                        staticResources.write(line.c_str(), line.size());
                    }
                    staticResources.close();
                    if(compareAndReplace(path + "temporary", path))
                    {
                        if (m_logLevel == shadercompiler::LogLevel::Recompile)
                            LOG_PURE("Updated static resources for %s", engine::pathExtractFilenameWithoutExtension(item.permutationPath).c_str());
                    }

                    if (engine::fileExists(path + "temporary"))
                        engine::fileDelete(path + "temporary");

                }
#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
                );
#endif
            }
#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
            );
#endif
        }
#ifdef MULTITHREADED_UPDATERESOURCEREGISTERS
        );
#endif
    }

    void CompileTask::compile(ShaderLocator& locator, bool optimization, GraphicsApi api)
    {
#ifdef MULTITHREADED_SHADERCOMPILE
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
            engine::string dxcPath;
            if(api == GraphicsApi::DX12)
                dxcPath = engine::pathClean("C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0/x86/dxc.exe");
            else if(api == GraphicsApi::Vulkan)
                dxcPath = engine::pathClean("C:/VulkanSDK/1.3.239.0/Bin/dxc.exe"); 

#ifdef MULTITHREADED_SHADERCOMPILE
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                if (stage.buildShaderBinary)
                {
                    auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
                    engine::string binaryTarget;
                    if (api == GraphicsApi::DX12)
                        binaryTarget = engine::pathJoin(locator.shaderBinaryPathDX12(), binFolderRelative);
                    else if (api == GraphicsApi::Vulkan)
                        binaryTarget = engine::pathJoin(locator.shaderBinaryPathVulkan(), binFolderRelative);

                    auto binaryTargetFolder = engine::pathExtractFolder(binaryTarget);

                    // make sure the directory is there
                    engine::Directory targetFolder(binaryTargetFolder);
                    if (!targetFolder.exists())
                        targetFolder.create();

                    // create root signature file path
                    auto rootSignaturePath = engine::pathReplaceExtension(binaryTarget, "rso");

                    bool dxrShader = false;
                    if (m_shaderProfiles[stage.name] == "lib_6_3")
                        dxrShader = true;

                    bool debug = !optimization;
                    engine::string outputFlags;
                    if (api == GraphicsApi::DX12)
                    {
                        if (debug)
                            outputFlags = "-nologo -Zpr -Od -Zi -Fo";
                        else
                            outputFlags = "-nologo -Zpr -O3 -Zi -Fo";
                    }
                    else if (api == GraphicsApi::Vulkan)
                    {
                        if (stage.name == "Vertex")
                        {
                            if (debug)
                                outputFlags = "-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O0 -Zi -Fo";
                            else
                                outputFlags = "-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O3 -Zi -Fo";
                        }
                        else
                        {
                            if (debug)
                                outputFlags = "-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O0 -Zi -Fo";
                            else
                                outputFlags = "-nologo -spirv -fspv-target-env=vulkan1.2 -Zpr -O3 -Zi -Fo";
                        }
                    }

                    auto writeSupportFile = [](
                        const engine::string& supportPath,
                        const engine::string& binary_file,
                        const engine::string& executable,
                        const engine::string& file,
                        const engine::string& graphics_api,
                        const engine::string& root_path,
                        const engine::string& shader_compiler_path,
                        const engine::string& shader_source_path,
                        const engine::vector<engine::string>& shaderIncludeDepencyPaths,
                        LogLevel logLevel)
                    {
                        engine::vector<engine::string> supportlines;
                        supportlines.emplace_back("{\n");
                        supportlines.emplace_back("    \"binary_file\": \"" + binary_file + "\",\n");
                        supportlines.emplace_back("    \"executable\": \"" + executable + "\",\n");
                        supportlines.emplace_back("    \"file\": \"" + file + "\",\n");
                        supportlines.emplace_back("    \"graphics_api\": \"" + graphics_api + "\",\n");
                        supportlines.emplace_back("    \"root_path\": \"" + root_path + "\",\n");
                        supportlines.emplace_back("    \"shader_compiler\": \"" + shader_compiler_path + "\",\n");
                        supportlines.emplace_back("    \"source_file\": \"" + shader_source_path + "\",\n");
                        
                        engine::string dependencyPaths = "    \"shader_include_depency_paths\": [\n";
                        for (auto dependencyPath = shaderIncludeDepencyPaths.begin(); dependencyPath != shaderIncludeDepencyPaths.end(); ++dependencyPath)
                            if (std::next(dependencyPath) != shaderIncludeDepencyPaths.end())
                                dependencyPaths += "        \"" + *dependencyPath + "\",\n";
                            else
                                dependencyPaths += "        \"" + *dependencyPath + "\"\n";
                        dependencyPaths += "    ]\n";
                        supportlines.emplace_back(dependencyPaths);

                        supportlines.emplace_back("}\n");

                        std::ofstream staticResources;
                        staticResources.open(supportPath + "temporary", std::ios::out);
                        ASSERT(staticResources.is_open(), "Cant output static resource content");
                        for (auto&& line : supportlines)
                        {
                            staticResources.write(line.c_str(), line.size());
                        }
                        staticResources.close();
                        if(compareAndReplace(supportPath + "temporary", supportPath))
                        {
                            if (logLevel == shadercompiler::LogLevel::Recompile)
                                LOG_PURE("Updated support file for %s", engine::pathExtractFilenameWithoutExtension(binary_file).c_str());
                        }
                    };

                    engine::string executable = R"foo(c:\\work\\darkness\\darkness-shadercompiler\\bin\\win64\\release\\darknessshadercompiler.exe)foo";
                    engine::string root_path = R"foo(C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderBuild.py)foo";
                    engine::string shader_compiler_path = R"foo(C:\\work\\darkness\\darkness-engine\\tools\\codegen\\ShaderCompiler.py)foo";

                    auto findAndReplaceAll = [](std::string& data, std::string toSearch, std::string replaceStr)
                    {
                        // Get the first occurrence
                        size_t pos = data.find(toSearch);
                        // Repeat till end is reached
                        while (pos != std::string::npos)
                        {
                            // Replace this occurrence of Sub String
                            data.replace(pos, toSearch.size(), replaceStr);
                            // Get the next occurrence from the current position
                            pos = data.find(toSearch, pos + replaceStr.size());
                        }
                    };

                    auto addSlash = [findAndReplaceAll](const engine::string& text)->engine::string
                    {
                        auto s = text;
                        findAndReplaceAll(s, "\\", "\\\\");
                        return s;
                    };

#ifdef MULTITHREADED_SHADERCOMPILE
                    std::for_each(
                        std::execution::par_unseq,
                        stage.items.begin(),
                        stage.items.end(),
                        [&](auto&& item)
#else
                    for (auto&& item : stage.items)
#endif
                    {
                        auto folder = engine::pathExtractFolder(binaryTarget);
                        auto permFile = engine::pathExtractFilename(item.permutationPath);
                        auto path = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "static_resources");
                        auto outputpath = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "cso");
                        auto outputsymbols = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "pdb");

                        engine::string arguments;
                        engine::string supportString;
                        if (api == GraphicsApi::DX12)
                        {
                            arguments = " /T " + m_shaderProfiles[stage.name] + " " + path + " " + outputFlags + " " + outputpath + " /Fd " + outputsymbols + " /setrootsignature " + rootSignaturePath;
                            supportString = "dx12";
                        }
                        else if (api == GraphicsApi::Vulkan)
                        {
                            arguments = " -T " + m_shaderProfiles[stage.name] + " " + path + " " + outputFlags + " " + outputpath;
                            supportString = "vulkan";
                        }

                        if (m_logLevel == LogLevel::Recompile) LOG_PURE("Compiling %s", outputpath.c_str());

                        engine::Process process(dxcPath, arguments, engine::pathExtractFolder(engine::getExecutableDirectory()), [&](const engine::string& msg)
                            {
                                auto cleanupMessage = [](const engine::string& msg)->engine::string
                                {
                                    auto lines = engine::tokenize(msg, { '\n' });
                                    int removeLast = 0;
                                    // remove uninteresting crud
                                    for (auto line = lines.rbegin(); line != lines.rend(); ++line)
                                    {
                                        if (line->empty() || 
                                            *line == "\n" || 
                                            *line == "\r" ||
                                            *line == "Exit code")
                                            ++removeLast;
                                        else
                                            break;
                                    }
                                    lines.erase(lines.end() - removeLast, lines.end());

                                    // remove too lengthy paths
                                    for (auto&& line : lines)
                                    {
                                        auto testStr = engine::string("C:\\work\\darkness\\darkness-engine\\data\\shaders\\");
                                        if (line.find(testStr) == 0)
                                            line = line.substr(testStr.length(), line.length() - testStr.length());
                                    }

                                    return engine::join(lines, '\n');
                                };
                                if (doLog(LogLevel::Error)) LOG_PURE("%s", cleanupMessage(msg).c_str());
                            });
                        writeSupportFile(
                            engine::pathReplaceExtension(outputpath, "support"),
                            addSlash(outputpath),
                            executable,
                            addSlash(stage.filename),
                            supportString,
                            root_path,
                            shader_compiler_path,
                            addSlash(item.sourcePath),
                            stage.shaderIncludeDepencyPaths,
                            m_logLevel);
                    }
#ifdef MULTITHREADED_SHADERCOMPILE
                    );
#endif
                }
            }// stage
#ifdef MULTITHREADED_SHADERCOMPILE
            );
#endif
        }
#ifdef MULTITHREADED_SHADERCOMPILE
        );
#endif
    }

    void CompileTask::patchVulkanRegisters(ShaderLocator& locator)
    {
#ifdef MULTITHREADED_PATCHVULKANREGISTERS
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
#ifdef MULTITHREADED_PATCHVULKANREGISTERS
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
                auto binaryTargetDX12 = engine::pathJoin(locator.shaderBinaryPathDX12(), binFolderRelative);
                auto binaryTargetFolderDX12 = engine::pathExtractFolder(binaryTargetDX12);
                auto binaryTargetVulkan = engine::pathJoin(locator.shaderBinaryPathVulkan(), binFolderRelative);
                auto binaryTargetFolderVulkan = engine::pathExtractFolder(binaryTargetVulkan);

                // make sure the directory is there
                engine::Directory targetFolder(binaryTargetFolderVulkan);
                if (!targetFolder.exists())
                    targetFolder.create();

#ifdef MULTITHREADED_PATCHVULKANREGISTERS
                std::for_each(
                    std::execution::par_unseq,
                    stage.items.begin(),
                    stage.items.end(),
                    [&](auto&& item)
#else
                for (auto&& item : stage.items)
#endif
                {
                    engine::vector<engine::string> lines;
                    // read DX12 preprocessed file
                    {
                        // create static resources file path
                        auto folder = engine::pathExtractFolder(binaryTargetDX12);
                        auto permFile = engine::pathExtractFilename(item.permutationPath);
                        engine::string path = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "static_resources");

                        // read preprocessed file
                        {
                            engine::vector<char> data = FileAccessSerializer::instance().readFile(path);
                            lines = readLines(data);
                        }
                    }

                    // produce Vulkan preprocessed file
                    {
                        // create static resources file path
                        auto folder = engine::pathExtractFolder(binaryTargetVulkan);
                        auto permFile = engine::pathExtractFilename(item.permutationPath);
                        engine::string path = engine::pathReplaceExtension(engine::pathJoin(folder, permFile), "static_resources");

                        // patch in vulkan bindings [[vk::binding(0,0)]]
                        std::sort(
                            item.vulkanBindings.begin(),
                            item.vulkanBindings.end(),
                            [](const ShaderPipelineStageItem::VulkanBinding& a, const ShaderPipelineStageItem::VulkanBinding& b)
                            {
                                return a.lineNumber > b.lineNumber;
                            });

                        for (auto&& v : item.vulkanBindings)
                        {
                            engine::string vkBind = "[[vk::binding(";
                            vkBind += std::to_string(v.bindingNumber);
                            vkBind += ",";
                            vkBind += std::to_string(v.setNumber);
                            vkBind += ")]]\n";
                            lines.insert(lines.begin() + v.lineNumber, vkBind);
                        }

                        // write out a new preprocessed file
                        {
                            std::ofstream staticResources;
                            staticResources.open(path + "temporary", std::ios::out);
                            ASSERT(staticResources.is_open(), "Cant output static resource content");
                            for (auto&& line : lines)
                            {
                                staticResources.write(line.c_str(), line.size());
                            }
                            staticResources.close();
                            if(compareAndReplace(path + "temporary", path))
                            {
                                //if (m_logLevel == shadercompiler::LogLevel::Recompile)
                                //    LOG_PURE("Updated vulkan preprocessed file for %s", engine::pathExtractFilenameWithoutExtension(path).c_str());
                            }
                        }
                    }
                }
#ifdef MULTITHREADED_PATCHVULKANREGISTERS
                );
#endif
            }
#ifdef MULTITHREADED_PATCHVULKANREGISTERS
            );
#endif
        }
#ifdef MULTITHREADED_PATCHVULKANREGISTERS
        );
#endif

    }

    void CompileTask::createCodeInterfaces(ShaderLocator& locator)
    {
#ifdef MULTITHREADED_CODEINTERFACES
        std::for_each(
            std::execution::par_unseq,
            locator.pipelines().begin(),
            locator.pipelines().end(),
            [&](auto&& pipeline)
#else
        for (auto&& pipeline : locator.pipelines())
#endif
        {
            engine::string someStageFilePath = "";
#ifdef MULTITHREADED_CODEINTERFACES
            std::for_each(
                std::execution::par_unseq,
                pipeline.stages.begin(),
                pipeline.stages.end(),
                [&](auto&& stage)
#else
            for (auto&& stage : pipeline.stages)
#endif
            {
                if (stage.buildCodeInterface)
                {
                    // get target binary file path
                    auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
                    auto binaryTarget = engine::pathJoin(locator.shaderInterfacePath(), binFolderRelative);
                    auto binaryTargetFolder = engine::pathExtractFolder(binaryTarget);

                    someStageFilePath = stage.filename;
                    // binaryTarget = "C:\\work\\darkness\\darkness-engine\\include\\shaders\\core\\shape\\RenderCones.vs.hlsl"

                    // make sure the directory is there
                    engine::Directory targetFolder(binaryTargetFolder);
                    if (!targetFolder.exists())
                        targetFolder.create();

                    auto hppFile = engine::pathReplaceExtension(binaryTarget, "h");
                    auto cppFile = engine::pathReplaceExtension(binaryTarget, "cpp");

                    if (doLog(LogLevel::All)) LOG_PURE("Creating C++ Load interfaces for %s", engine::pathJoin(binFolderRelative, engine::pathExtractFilename(stage.filename)).c_str());

                    shadercompiler::TemplateProcessor::ProcessShaderLoadInterfaces(
                        locator,
                        stage.filename,
                        locator.shaderLoadInterfaceTemplateHeader(),
                        hppFile,
                        stage,
                        m_logLevel);

                    shadercompiler::TemplateProcessor::ProcessShaderLoadInterfaces(
                        locator,
                        stage.filename,
                        locator.shaderLoadInterfaceTemplateSource(),
                        cppFile,
                        stage,
                        m_logLevel);
                }
            }
#ifdef MULTITHREADED_CODEINTERFACES
            );
#endif
            if (pipeline.buildCodeInterface)
            {
                auto binFolderRelative = someStageFilePath.substr(locator.shaderRootPath().length(), someStageFilePath.length() - locator.shaderRootPath().length());
                auto binaryTarget = engine::pathJoin(locator.shaderInterfacePath(), binFolderRelative);
                auto binaryTargetFolder = engine::pathExtractFolder(binaryTarget);
                auto pipelineTarget = engine::pathExtractFilenameWithoutExtension(binaryTarget);

                auto hppFile = engine::pathJoin(engine::pathExtractFolder(binaryTarget), engine::pathReplaceExtension(pipelineTarget, "h"));
                auto cppFile = engine::pathJoin(engine::pathExtractFolder(binaryTarget), engine::pathReplaceExtension(pipelineTarget, "cpp"));

                if (doLog(LogLevel::All)) LOG_PURE("Creating C++ Pipeline interfaces for %s", engine::pathJoin(binFolderRelative, pipeline.pipelineName).c_str());

                shadercompiler::TemplateProcessor::ProcessPipelineInterfaces(
                    locator.shaderPipelineInterfaceTemplateHeader(),
                    pipeline,
                    hppFile,
                    m_logLevel);

                shadercompiler::TemplateProcessor::ProcessPipelineInterfaces(
                    locator.shaderPipelineInterfaceTemplateSource(),
                    pipeline,
                    cppFile,
                    m_logLevel);
            }
        }
#ifdef MULTITHREADED_CODEINTERFACES
        );
#endif
    }
}
