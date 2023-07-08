#pragma once

#include "containers/string.h"
#include "containers/unordered_map.h"
#include "containers/vector.h"

namespace engine
{
    class ArgParser
    {
    public:
        ArgParser(int argc, char **argv);
        engine::string value(const engine::string& key);
        bool isSet(const engine::string& key);
        bool flag(const engine::string& flag) const;
        const engine::vector<engine::string>& flags() const;
    private:
        engine::string m_pathOfExecutable;
        void parseArg(const engine::string& arg);
        engine::unordered_map<engine::string, engine::string> m_values;
        engine::vector<engine::string> m_flags;
    };
}
