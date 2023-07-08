#include "engine/graphics/ShaderSupport.h"

#include <iostream>
#include <fstream>
#include "containers/vector.h"

using namespace engine;

#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace rapidjson;

namespace engine
{
    namespace implementation
    {
        ShaderSupport::ShaderSupport(const engine::string& supportFilePath)
        {
            // read support
			std::ifstream supportFile;
            supportFile.open(supportFilePath.c_str(), std::ios::in);
            if (supportFile.is_open())
            {
                auto begin = supportFile.tellg();
                supportFile.seekg(0, std::ios::end);
                auto end = supportFile.tellg();
                size_t size = static_cast<size_t>(end - begin);

                engine::vector<char> supportData(size);

                supportFile.seekg(0, std::ios::beg);
                supportFile.read(supportData.data(), end - begin);
                supportFile.close();

                StringStream ss(supportData.data());
                Document document;
                document.ParseStream(ss);

                auto propertyString = [&document](const char* option, engine::string& target)
                {
                    if (document.HasMember(option) && document[option].IsString())
                    {
                        target = document[option].GetString();
                    }
                    return "";
                };
                propertyString("binary_file", binaryFile);
                propertyString("executable", executable);
                propertyString("file", file);
                propertyString("graphics_api", graphicsApi);
                propertyString("shader_compiler_path", shaderCompilerPath);
                propertyString("root_path", rootPath);
                propertyString("source_file", sourceFile);

                if (document.HasMember("shader_include_depency_paths"))
                {
                    const rapidjson::Value& dependencyPaths = document["shader_include_depency_paths"];
                    if (dependencyPaths.IsArray())
                    {
                        for (auto path = dependencyPaths.Begin(); path != dependencyPaths.End(); ++path)
                            includeFiles.emplace_back(path->GetString());
                    }
                }
                
            }
        }
    }
}
