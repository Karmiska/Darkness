#include "tools/ArgParser.h"
#include "tools/StringTools.h"
#include "tools/Debug.h"

namespace engine
{
    ArgParser::ArgParser(int argc, char **argv)
    {
        m_pathOfExecutable = engine::string(argv[0]);
        for (int i = 1; i < argc; ++i)
        {
            parseArg(argv[i]);
        }
    }

    void ArgParser::parseArg(const engine::string& arg)
    {
        auto tokens = tokenize(arg, { '=' });
        if (tokens.size() == 1)
        {
            auto token = tokens[0];
            while (token.find('-') == 0)
                token = token.substr(1, token.length() - 1);
            m_flags.emplace_back(token);
        }
        else if (tokens.size() == 2)
        {
            auto token = tokens[0];
            while (token.find('-') == 0)
                token = token.substr(1, token.length() - 1);

            m_values[token] = tokens[1];
        }
        else
        {
            LOG_ERROR("weird argument: %s", arg.c_str());
        }
    }

    engine::string ArgParser::value(const engine::string& key)
    {
        if(isSet(key))
            return m_values[key];
        return "";
    }

    bool ArgParser::isSet(const engine::string& key)
    {
        return m_values.find(key) != m_values.end();
    }

    bool ArgParser::flag(const engine::string& flag) const
    {
        for (auto&& f : m_flags)
        {
            if (f == flag)
                return true;
        }
        return false;
    }

    const engine::vector<engine::string>& ArgParser::flags() const
    {
        return m_flags;
    }
}
