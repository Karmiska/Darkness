#include "tools/StringTools.h"
#include <string>

namespace engine
{
    engine::vector<engine::string> tokenize(engine::string str, const engine::vector<char>& delims)
    {
        engine::vector<engine::string> result;
        for_each_token(
            std::cbegin(str), std::cend(str),
            std::cbegin(delims), std::cend(delims),
            [&result](auto first, auto second)
        {
            if (first != second)
                result.emplace_back(engine::string(first, second));
        }
        );
        return result;
    }

    engine::string join(const engine::vector<engine::string>& strings)
    {
        engine::string result;
        for (auto&& str : strings)
            result += str;
        return result;
    }

    engine::string join(const engine::vector<engine::string>& strings, const char delim)
    {
        if (strings.empty())
            return "";
        engine::string result;
        auto size = strings.size();
        for (int i = 0; i < size - 1; ++i)
        {
            result += strings[i];
            result += delim;
        }
        result += strings[size - 1];
        return result;
    }

	template<>
	float stringToNumber<float>(const engine::string& value)
	{
		std::string temp = value.c_str();
		float result;
		std::stringstream stream(temp);
		stream >> result;
		ASSERT(!stream.fail());
		return result;
	}

	template<>
	int stringToNumber<int>(const engine::string& value)
	{
		std::string temp = value.c_str();
		int result;
		std::stringstream stream(temp);
		stream >> result;
		ASSERT(!stream.fail());
		return result;
	}
}
