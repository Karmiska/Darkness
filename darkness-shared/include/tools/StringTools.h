#pragma once

#include "containers/string.h"
#include <sstream>
#include "containers/vector.h"
#include <algorithm>
#include "tools/Debug.h"

namespace engine
{
    static const char DefaultDelimiters[] = { ":" };
    engine::vector<engine::string> tokenize(
        engine::string str, 
        const engine::vector<char>& delims = engine::vector<char>(
            DefaultDelimiters, 
            DefaultDelimiters + sizeof(DefaultDelimiters) / sizeof(DefaultDelimiters[0])));

    template <class InputIt, class ForwardIt, class BinOp>
    void for_each_token(InputIt first, InputIt last, ForwardIt s_first, ForwardIt s_last, BinOp binaryOp)
    {
        while (first != last)
        {
            const auto pos = std::find_first_of(first, last, s_first, s_last);
            binaryOp(first, pos);
            if (pos == last) break;
            first = std::next(pos);
        }
    }

    engine::string join(const engine::vector<engine::string>& strings);
    engine::string join(const engine::vector<engine::string>& strings, const char delim);

    template<typename T>
	T stringToNumber(const engine::string& value);
    
    std::wstring toWideString(const engine::string& str);
    engine::string toUtf8String(const std::wstring& str);
}
