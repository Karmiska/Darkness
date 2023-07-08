#include "Preprocessor.h"
#include "tools/Process.h"
#include "tools/PathTools.h"
#include "platform/Environment.h"
#include "platform/File.h"
#include "ShaderCompilerCommon.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include "containers/unordered_map.h"
#include "Helpers.h"

#define SHADER_SOURCE_PREPROCESS

namespace shadercompiler
{
    engine::vector<engine::string> readLines(const engine::vector<char>& data)
    {
        engine::vector<engine::string> result;
        std::stringstream ss(data.data());
        std::string to;
        while (std::getline(ss, to, '\n'))
            result.emplace_back(to + '\n');
        return result;
    }

    engine::vector<engine::string> readLinesRecursive(const engine::string& file)
    {
        engine::vector<char> data = FileAccessSerializer::instance().readFile(file);

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

                auto includeLines = readLinesRecursive(absolutePath);
                for (auto&& l : includeLines)
                    result.emplace_back(l);
            }
            else
                result.emplace_back(line);
        }
        return result;
    }

    Preprocessor::Preprocessor(const engine::string& filepath)
    {
        auto lines = readLinesRecursive(filepath);
        for (auto&& line : lines)
            checkForPermutations(line);
    }

    engine::vector<engine::string> Preprocessor::process(
        const engine::string& filepath,
        const engine::vector<engine::string>& includePaths,
        const engine::vector<engine::string>& defines,
        const engine::string& outputPath,
        engine::vector<engine::string>& shaderIncludeDepencyPaths) const
    {
        auto dxcPath = engine::pathClean("C:/Program Files (x86)/Windows Kits/10/bin/10.0.19041.0/x86/dxc.exe");

        auto shaSeed = outputPath;
        for (auto&& define : defines)
            shaSeed += define;

        auto tempPathHash = sha1(shaSeed);
        auto tempPath = engine::pathReplaceExtension(filepath, engine::pathExtractExtension(filepath) + tempPathHash);
        engine::string arguments = "-P " + tempPath + " ";
        arguments += filepath;
        for (auto&& def : defines)
        {
            arguments += " -D" + def;
        }
        

        {
            // in it's own scope so we wait for the execution to finish
            bool done = false;
            int error = 5;
            while (!done)
            {
                done = true;
                engine::Process process(dxcPath, arguments, engine::pathExtractFolder(engine::getExecutableDirectory()), [&](const engine::string& msg)
                    {
                        if (msg.find("The process cannot access the file because it is being used by another process") != engine::string::npos)
                        {
                            // just do it again
                            done = false;
                        }
                        else
                        {
                            done = false;
                            --error;
                            if (error == 0)
                            {
                                LOG_PURE("Msg from %s, Preprocessors message: %s",
                                    filepath.c_str(),
                                    msg.c_str());
                            }
                        }
                    });

                if (error == 0)
                    break;

                if (!done)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        // read preprocessed result
        engine::vector<char> data;
        {
            data = FileAccessSerializer::instance().readFile(tempPath);

            if (engine::fileExists(tempPath))
                engine::fileDelete(tempPath);
        }

        auto lines = readLines(data);
        auto dataSize = removePreprocessorIncludes(filepath, lines, shaderIncludeDepencyPaths);

        /*engine::string m_data;
        m_data.resize(dataSize);
        auto ptr = m_data.data();
        int index = 0;
        for (auto&& line : lines)
        {
            memcpy(ptr, line.data(), line.size());
            ptr += line.size();
        }

        // write out preprocessed file
        std::ofstream finalpreprocessedFile;
        finalpreprocessedFile.open(outputPath, std::ios::out);
        if (finalpreprocessedFile.is_open())
        {
            finalpreprocessedFile.write(m_data.data(), m_data.size());
            finalpreprocessedFile.close();
        }*/

        return lines;
    }

    engine::vector<engine::string> Preprocessor::readLines(const engine::vector<char>& data) const
    {
        engine::vector<engine::string> lines;
        std::stringstream ss(data.data());
        std::string to;
        while (std::getline(ss, to, '\n'))
            lines.emplace_back(to + '\n');
        return lines;
    }

    int Preprocessor::removePreprocessorIncludes(
        const engine::string& thisFile,
        engine::vector<engine::string>& lines,
        engine::vector<engine::string>& shaderIncludeDepencyPaths) const
    {
        int dataSize = 0;
        for (auto line = lines.begin(); line != lines.end();)
        {
            if ((*line).find("#line") == 0)
            {
                if ((*line).find("#line 1 ") == 0)
                {
                    // absolute include path. first occurrence.
                    // #line 1 "C:\\work\\ . . . \\RenderClustersGbufferAlphaClipped.ps.hlsl"
                    auto quoteStart = (*line).find("\"") + 1;
                    shaderIncludeDepencyPaths.emplace_back((*line).substr(quoteStart, (*line).length() - quoteStart - 2));
                }
                auto lineTemp = line;
                line = lines.erase(lineTemp);
            }
            else
            {
                dataSize += (*line).size();
                ++line;
            }
        }

        // cleanup the paths
        for (auto&& path : shaderIncludeDepencyPaths)
            path = engine::pathReplaceAllDelimiters(engine::pathClean(path), "\\\\");

        // remove duplicates
        std::sort(
            shaderIncludeDepencyPaths.begin(),
            shaderIncludeDepencyPaths.end());
        shaderIncludeDepencyPaths.erase(
            std::unique(
                shaderIncludeDepencyPaths.begin(),
                shaderIncludeDepencyPaths.end()),
            shaderIncludeDepencyPaths.end());

        // remove self
        auto cleanedOrigFilePath = engine::pathReplaceAllDelimiters(engine::pathClean(thisFile), "\\\\");
        shaderIncludeDepencyPaths.erase(std::remove_if(
            shaderIncludeDepencyPaths.begin(),
            shaderIncludeDepencyPaths.end(),
            [&](const engine::string& path)->bool
            {
                return path == cleanedOrigFilePath;
            }), shaderIncludeDepencyPaths.end());

        // remove empty
        shaderIncludeDepencyPaths.erase(std::remove_if(
            shaderIncludeDepencyPaths.begin(),
            shaderIncludeDepencyPaths.end(),
            [&](const engine::string& path)->bool
            {
                return path == "";
            }), shaderIncludeDepencyPaths.end());

        return dataSize;
    }

    void Preprocessor::checkForPermutations(const engine::string& line)
    {
        auto index = line.find("ENUM_");
        if (index != engine::string::npos)
        {
            // define == ENUM_MODE_FILL_ALL
            auto define = engine::tokenize(line.substr(index, line.length() - index), { ' ', '\n' })[0];

            // enumname == MODE
            auto enumname = engine::tokenize(define.substr(5, define.length()-5), { '_' })[0];
            // enumname == Mode
            std::transform(enumname.begin()+1, enumname.end(), enumname.begin()+1, ::tolower);

            // enumvalue == FILL_ALL
            auto enumvalue = define.substr(5 + enumname.length() + 1, define.length() - (5 + enumname.length() + 1));

            // valueparts == FILL, ALL
            auto valueparts = engine::tokenize(enumvalue, { '_' });

            // finalValue == FillAll
            engine::string finalValue;
            for (auto&& vp : valueparts)
            {
                std::transform(vp.begin() + 1, vp.end(), vp.begin() + 1, ::tolower);
                finalValue += vp;

            }

            auto mexists = m_enums.find(enumname);
            if (mexists == m_enums.end())
            {
                m_enums[enumname] = {};
                mexists = m_enums.find(enumname);
            }

            auto exists = std::find_if((*mexists).second.begin(), (*mexists).second.end(), [&](const Condition& cond) { return cond.value == finalValue; });
            if (exists == (*mexists).second.end())
                (*mexists).second.emplace_back(Condition{ finalValue, define });
        }

        index = line.find("OPTION_");
        if (index != engine::string::npos)
        {
            // define == OPTION_DEBUG_THING
            auto define = engine::tokenize(line.substr(index, line.length() - index), { ' ', '\n' })[0];

            // value == DEBUG_THING
            auto value = define.substr(7, define.length() - 7);
            
            // valueparts == DEBUG, THING
            auto valueparts = engine::tokenize(value, { '_' });

            // finalValue == DebugThing
            engine::string finalValue;
            for (auto&& vp : valueparts)
            {
                std::transform(vp.begin() + 1, vp.end(), vp.begin() + 1, ::tolower);
                finalValue += vp;
                    
            }

            auto exists = std::find_if(m_options.begin(), m_options.end(), [&](const Condition& cond) { return cond.value == finalValue; });
            if (exists == m_options.end())
                m_options.emplace_back(Condition{ finalValue, define });
        }
    }
}