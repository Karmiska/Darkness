#pragma once

#include "containers/string.h"
#include "tools/Settings.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    class ShaderLocator
    {
    public:
        static ShaderLocator& instance() { static ShaderLocator locator; return locator; }

        string getCoreShaderPath() const
        {
            return getCoreShaderPath(m_defaultApi);
        }

        string getCoreShaderPath(GraphicsApi api) const
        {
            string shaderPath = m_settings.get<string>("rootShaderPath");
            auto repl = shaderPath.find("$DATA_ROOT");
            if (repl != engine::string::npos)
            {
                auto dataRootStrLen = string("$DATA_ROOT\\").length();
                auto dataRoot = engine::pathClean(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()),
                    shaderPath.substr(dataRootStrLen, shaderPath.length() - dataRootStrLen)));
                    //"..\\..\\..\\data"));
                shaderPath = dataRoot;// .replace(repl, 10, dataRoot);
            }

            string apiPath;
            if (api == GraphicsApi::DX12)
                apiPath = m_settings.get<string>("apiShaderPathDX12");
            else if (api == GraphicsApi::Vulkan)
                apiPath = m_settings.get<string>("apiShaderPathVulkan");

            string corePath = m_settings.get<string>("coreShaderPath");

            return pathClean(pathJoin(pathJoin(shaderPath, apiPath), corePath));
        }

        GraphicsApi defaultApi() const { return m_defaultApi; }
        void defaultApi(GraphicsApi api) { m_defaultApi = api; }
    private:
        ShaderLocator()
            : m_settings{ pathClean(pathJoin(pathJoin(pathExtractFolder(getExecutableDirectory()), "../../../data/"), "ShaderLocations.json")) }
            , m_defaultApi{ GraphicsApi::DX12 }
        {

        }

        tools::Settings m_settings;
        GraphicsApi m_defaultApi;
    };
}
