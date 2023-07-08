#pragma once

#include "containers/string.h"
#include "containers/vector.h"
#include "containers/unordered_map.h"

namespace shadercompiler
{
    engine::vector<engine::string> readLines(const engine::vector<char>& data);

    struct Condition
    {
        engine::string value;    // debug, red
        engine::string define;   // OPTION_DEBUG, ENUM_COLOR_RED
    };

    class Preprocessor
    {
    public:
        Preprocessor(const engine::string& filepath);

        engine::vector<engine::string> process(
            const engine::string& filepath,
            const engine::vector<engine::string>& includePaths,
            const engine::vector<engine::string>& defines,
            const engine::string& outputPath,
            engine::vector<engine::string>& shaderIncludeDepencyPaths) const;

        const engine::vector<Condition>& options() const
        {
            return m_options;
        }

        const engine::unordered_map<engine::string, engine::vector<Condition>>& enums() const
        {
            return m_enums;
        }
    private:
        engine::vector<engine::string> readLines(const engine::vector<char>& data) const;
        int removePreprocessorIncludes(
            const engine::string& thisFile,
            engine::vector<engine::string>& lines,
            engine::vector<engine::string>& shaderIncludeDepencyPaths) const;
        void checkForPermutations(const engine::string& line);

        // with OPTION_DEBUG =   debug, OPTION_DEBUG
        engine::vector<Condition> m_options;

        // with ENUM_COLOR_RED =   COLOR               red, ENUM_COLOR_RED
        engine::unordered_map<engine::string, engine::vector<Condition>> m_enums;

    };
}
