#pragma once
#if 0
#include "containers/string.h"
#include "containers/vector.h"

namespace shadercompiler
{
    class LexicalAnalyzer
    {
    public:
        struct Token
        {
            engine::string value;
            int linenumber;
            engine::string type;
        };

        LexicalAnalyzer(const engine::string& data);

	private:
		engine::vector<Token> m_tokens;
    };
}
#endif
