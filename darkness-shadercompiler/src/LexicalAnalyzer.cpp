#include "LexicalAnalyzer.h"
#if 0
namespace shadercompiler
{
    const char* Qualifiers[] = { 
        "static",
        "const",
        "volatile" };

    const char* CompleteSystemTypes[] = {
		"void",
		"bool",
		"int",		// 32 bit signed integer
		"uint",		// 32 bit unsigned integer
		"dword",	// 32 bit unsigned integer
		"half",		// 16 bit floating point
		"float",	// 32 bit floating point
		"double",	// 64 bit floating point
		
		"int1",	  "int2",   "int3",   "int4",
		"int1x1", "int2x1", "int3x1", "int4x1",
		"int1x2", "int2x2", "int3x2", "int4x2",
		"int1x3", "int2x3", "int3x3", "int4x3",
		"int1x4", "int2x4", "int3x4", "int4x4",
		
		"uint1",   "uint2",   "uint3",   "uint4",
		"uint1x1", "uint2x1", "uint3x1", "uint4x1",
		"uint1x2", "uint2x2", "uint3x2", "uint4x2",
		"uint1x3", "uint2x3", "uint3x3", "uint4x3",
		"uint1x4", "uint2x4", "uint3x4", "uint4x4",
		
		"float1",   "float2",   "float3",   "float4",
		"float1x1", "float2x1",	"float3x1",	"float4x1",
		"float1x2",	"float2x2",	"float3x2",	"float4x2",
		"float1x3",	"float2x3",	"float3x3",	"float4x3",
		"float1x4",	"float2x4",	"float3x4",	"float4x4",
		
		"double1",   "double2",   "double3",   "double4",
		"double1x1", "double2x1", "double3x1", "double4x1",
		"double1x2", "double2x2", "double3x2", "double4x2",
		"double1x3", "double2x3", "double3x3", "double4x3",
		"double1x4", "double2x4", "double3x4", "double4x4",
		
		"texture",
		"sampler",
		"SamplerComparisonState",
		"RaytracingAccelerationStructure",
		"ByteAddressBuffer" };

	const char* TemplatedSystemTypes[] = {
		"Buffer",	// Buffer < float4>
		"Texture1D",
		"Texture1DArray",
		"Texture2D",
		"Texture2DArray",
		"Texture3D",
		"TextureCube",
		"TextureCubeArray",
		
		"Texture2DMS",
		"Texture2DMSArray",
		
		"RWBuffer",
		"RWByteAddressBuffer",
		"RWStructuredBuffer",
		"AppendStructuredBuffer",
		"RWTexture1D",
		"RWTexture1DArray",
		"RWTexture2D",
		"RWTexture2DArray",
		"RWTexture3D",
		
		"StructuredBuffer",
		"ConstantBuffer" };

	std::pair<const char*, const char*> Operators[] = {
	//std::pair<const char*, const char*> ArithmeticOperators[] = {
		{ "=", "assignment" },
		{ "+", "addition" },
		{ "-", "subtraction" },
		// unary+
		// unary-
		{ "*", "multiplication" },
		{ "/", "division" },
		{ "%", "modulo" },
		{ "++", "increment" },
		{ "--", "decrement" },
	//};

	//std::pair<const char*, const char*> ComparisonOperators[] = {
		{ "==", "equal" },
		{ "!=", "not_equal" },
		{ ">", "greater" },
		{ "<", "lesser" },
		{ ">=", "greater_equal" },
		{ "<=", "lesser_equal" },
	//};

	//std::pair<const char*, const char*> LogicalOperators[] = {
		{ "!", "logical_not" },
		{ "&&", "logical_and" },
		{ "||", "logical_or" },
	//};

	//std::pair<const char*, const char*> BitwiseOperators[] = {
		{ "~", "bitwise_not" },
		{ "&", "bitwise_and" },
		{ "|", "bitwise_or" },
		{ "^", "bitwise_xor" },
		{ "<<", "bitwise_left_shift" },
		{ ">>", "bitwise_right_shift" },
	//};

	//std::pair<const char*, const char*> compound_assignment_operators[] = {
		{ "+=", "addition_assignment" },
		{ "-=", "subtraction_assignment" },
		{ "*=", "multiplication_assignment" },
		{ "/=", "division_assignment" },
		{ "%=", "modulo_assignment" },
		{ "&=", "bitwise_and_assignment" },
		{ "|=", "bitwise_or_assignment" },
		{ "^=", "bitwise_xor_assignment" },
		{ "<<=", "bitwise_left_shift_assignment" },
		{ ">>=", "bitwise_right_shift_assignment" }
	};

    LexicalAnalyzer::LexicalAnalyzer(const engine::string& data)
    {
        auto validIdentifierFirstCharacter = [](char c)->bool
        {
            return isalpha(c) || c == '_';
        };

        auto validIdentifierCharacter = [](char c)->bool
        {
            return isalpha(c) || isdigit(c) || c == '_';
        };

        auto inQualifiers = [](const engine::string& s)->bool
        {
            for (auto&& q : Qualifiers)
                if (engine::string(q) == s)
                    return true;
            return false;
        };

        auto inCompleteSystemTypes = [](const engine::string& s)->bool
        {
            for (auto&& q : CompleteSystemTypes)
                if (engine::string(q) == s)
                    return true;
            return false;
        };

        auto inTemplatedSystemTypes = [](const engine::string& s)->bool
        {
            for (auto&& q : TemplatedSystemTypes)
                if (engine::string(q) == s)
                    return true;
            return false;
        };

        auto isOperatorCharacter = [](char c)->bool
        {
            return
                c == '=' ||
                c == '+' ||
                c == '-' ||
                c == '*' ||
                c == '/' ||
                c == '%' ||
                c == '!' ||
                c == '>' ||
                c == '<' ||
                c == '&' ||
                c == '|' ||
                c == '^' ||
                c == '~';
        };

        auto maxOperatorLength = 3;
        auto operatorCount = sizeof(Operators) / sizeof(*Operators);

        int lineNumber = 0;
        engine::string lastTokenType = "";
        for (auto chr = data.begin(); chr != data.end(); ++chr)
        {
            Token token;

            if ((*chr) == '\n')
                ++lineNumber;

            token.linenumber = lineNumber;

            if (validIdentifierFirstCharacter((*chr)))
            {
                engine::string word;
                word += (*chr);
                ++chr;
                while (validIdentifierCharacter((*chr)))
                {
                    word += (*chr);
                    ++chr;
                }
                token.value = word;
                if (inQualifiers(token.value))
                    token.type = "qualifier";
                else if (inCompleteSystemTypes(token.value) || inTemplatedSystemTypes(token.value))
                    token.type = "system_type";
                else
                    token.type = "identifier";

                --chr;

                m_tokens.emplace_back(token);
            }

            // parse pre negation operator
            bool negativeNumber = false;
            if ((*chr) == '-' && lastTokenType != "number")
            {
                negativeNumber = true;
                ++chr;
                while ((*chr) == ' ')
                    ++chr;
            }
            if (negativeNumber)
                if (!isdigit(*chr))
                    --chr;

            // parse numbers
            if (isdigit(*chr))
            {
                engine::string number;
                number += *chr;
                ++chr;
                while (isdigit(*chr) || (*chr) == '.' || (*chr) == 'f')
                {
                    number += *chr;
                    ++chr;
                }
                if (negativeNumber)
                    token.value = "-" + number;
                else
                    token.value = number;
                token.type = "number";
                --chr;
                m_tokens.emplace_back(token);
            }
            if (*chr == '.')
            {
                ++chr;
                if (isdigit(*chr))
                {
                    engine::string number;
                    number += "0.";
                    number += *chr;
                    ++chr;
                    while (isdigit(*chr) || (*chr) == 'f')
                    {
                        number += *chr;
                        ++chr;
                    }
                    if (negativeNumber)
                        token.value = "-" + number;
                    else
                        token.value = number;

                    token.type = "number";
                    --chr;
                    m_tokens.emplace_back(token);
                }
            }

            // parse operators
            engine::string characters;
            int receivedCharacters = 0;
            for (int i = 0; i < maxOperatorLength; ++i)
            {
                if (isOperatorCharacter(*chr))
                {
                    characters += *chr;
                    ++receivedCharacters;
                }
                else
                    break;
            }
            for (int i = 0; i < receivedCharacters; ++i)
                --chr;

            bool found = false;
            if(receivedCharacters > 0)
                for(int i = operatorCount-1; i >= 0; --i)
                    if (Operators[i].first == characters)
                    {
                        token.value = Operators[i].second;
                        token.type = "operator";
                        found = true;
                        break;
                    }
            if (found)
                m_tokens.emplace_back(token);


            // parse other operators
			switch(*chr)
            {
                case '.':
                {
                    token.value = "dot_operator";
                    token.type = "operator";
                    m_tokens.emplace_back(token);
                    break;
                }
                case ',':
                {
                    token.value = "comma_operator";
                    token.type = "operator";
                    m_tokens.emplace_back(token);
                    break;
                }
                case '(':
                {
                    token.value = "left_parentheses";
                    token.type = "parentheses";
                    m_tokens.emplace_back(token);
                    break;
                }
                case ')':
                {
                    token.value = "right_parentheses";
                    token.type = "parentheses";
                    m_tokens.emplace_back(token);
                    break;
                }
                case '[':
                {
                    token.value = "left_bracket";
                    token.type = "bracket";
                    m_tokens.emplace_back(token);
                    break;
                }
			    case ']':
                {
                    token.value = "right_bracket";
                    token.type = "bracket";
                    m_tokens.emplace_back(token);
                    break;
                }
			    case '{':
                {
                    token.value = "left_brace";
                    token.type = "brace";
                    m_tokens.emplace_back(token);
                    break;
                }
			    case '}':
                {
                    token.value = "right_brace";
                    token.type = "brace";
                    m_tokens.emplace_back(token);
                    break;
                }
			    case ';':
                {
                    token.value = "semicolon";
                    token.type = "semicolon";
                    m_tokens.emplace_back(token);
                    break;
                }
			    case ':':
                {
                    token.value = "colon";
                    token.type = "colon";
                    m_tokens.emplace_back(token);
                    break;
                }
            }
        }
    }
}
#endif