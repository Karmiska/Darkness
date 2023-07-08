#include "ResourceMap.h"
#include "tools/Debug.h"
#include "tools/PathTools.h"

namespace shadercompiler
{
	const char* ReadTextureTypes[] = {
		"Texture2DMSArray",
		"TextureCubeArray",
		"Texture1DArray",
		"Texture2DArray",
		"Texture2DMS",
		"TextureCube",
		"Texture1D",
		"Texture2D",
		"Texture3D",
	};

	const char* WriteTextureTypes[] = {
		"RWTexture1DArray",
		"RWTexture2DArray",
		"RWTexture1D",
		"RWTexture2D",
		"RWTexture3D",
	};

	const char* ReadBufferTypes[] = {
		"ByteAddressBuffer",
		"Buffer",	// Buffer < float4>
	};

	const char* WriteBufferTypes[] = {
		"RWByteAddressBuffer",
		"RWStructuredBuffer",
		"RWBuffer",
	};

	const char* SpecialTypes[] = {
		"RaytracingAccelerationStructure",
		"AppendStructuredBuffer",
		"StructuredBuffer",
		"ConstantBuffer",
	};

	const char* CompleteSystemTypes[] = {
		// 0, 3
		"void",
		"bool",
		"half", // 16 bit floating point

		// 3, 21
		"int",
		"int1", "int2", "int3", "int4",
		"int1x1", "int2x1", "int3x1", "int4x1",
		"int1x2", "int2x2", "int3x2", "int4x2",
		"int1x3", "int2x3", "int3x3", "int4x3",
		"int1x4", "int2x4", "int3x4", "int4x4",

		// 24, 21
		"uint",
		"uint1", "uint2", "uint3", "uint4",
		"uint1x1", "uint2x1", "uint3x1", "uint4x1",
		"uint1x2", "uint2x2", "uint3x2", "uint4x2",
		"uint1x3", "uint2x3", "uint3x3", "uint4x3",
		"uint1x4", "uint2x4", "uint3x4", "uint4x4",
		
		// 45, 21
		"float",
		"float1", "float2", "float3", "float4",
		"float1x1", "float2x1", "float3x1", "float4x1",
		"float1x2", "float2x2", "float3x2", "float4x2",
		"float1x3", "float2x3", "float3x3", "float4x3",
		"float1x4", "float2x4", "float3x4", "float4x4",
		
		// 66, 22
		"double", "dword",
		"double1", "double2", "double3", "double4",
		"double1x1", "double2x1", "double3x1", "double4x1",
		"double1x2", "double2x2", "double3x2", "double4x2",
		"double1x3", "double2x3", "double3x3", "double4x3",
		"double1x4", "double2x4", "double3x4", "double4x4",
		
		// 88
		"texture",
		"sampler",
		"SamplerComparisonState",
		"RaytracingAccelerationStructure",
		"ByteAddressBuffer"
	};

	const char* AllTemplatedSystemTypes[] = {
		"Buffer", // Buffer < float4>
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
		"ConstantBuffer"
	};

    ResourceMap::ResourceMap(engine::vector<engine::string>& lines)
    {
		struct TemplateTypeAndName
		{
			engine::string type;
			engine::string name;
			bool bindless;
		};
		auto parseTemplateTypeAndName = [&](const engine::string& line, int index)->TemplateTypeAndName
		{
			TemplateTypeAndName result;

			auto tempMarkerIn = line.find('<', index);
			auto tempMarkerOut = line.find('>', tempMarkerIn);
			
			result.type = line.substr(tempMarkerIn+1, tempMarkerOut - tempMarkerIn - 1);

			// jump over >
			++tempMarkerOut;

			// jump over empty space
			while (line[tempMarkerOut] == ' ')
				++tempMarkerOut;

			auto nameStart = tempMarkerOut;
			// jump forward
			while (line[tempMarkerOut] != '[' && line[tempMarkerOut] != ';')
				++tempMarkerOut;
			auto nameStop = tempMarkerOut;

			result.name = line.substr(nameStart, nameStop - nameStart);
			result.bindless = line[tempMarkerOut] == '[';

			return result;
		};
		auto inTemplatedSystemTypes = [&](const engine::string& line, ResourceMapping& tempType)->bool
		{
			auto isStructured = [](const engine::string& format)->bool
			{
				return format.find("Structured") != engine::string::npos;
			};

			if (line.size() > 3)
			{
				engine::string piece2{ line[0], line[1] };
				engine::string piece3 = piece2 + line[2];
				if (piece2 == "Te")
				{
					for (auto&& type : ReadTextureTypes)
					{
						if (line.substr(0, strlen(type)) == type)
						{
							auto tt = parseTemplateTypeAndName(line, strlen(type));
							tempType.name = tt.name;
							tempType.type = ResourceType::Texture;
							tempType.access = ResourceAccess::Read;
							tempType.templatedType = tt.type;
							tempType.bindless = tt.bindless;
							tempType.dimension = type;
							tempType.structured = false;
							return true;
						}
					}
				}
				else if (piece3 == "RWT")
				{
					for (auto&& type : WriteTextureTypes)
					{
						if (line.substr(0, strlen(type)) == type)
						{
							auto tt = parseTemplateTypeAndName(line, strlen(type));
							tempType.name = tt.name;
							tempType.type = ResourceType::Texture;
							tempType.access = ResourceAccess::Write;
							tempType.templatedType = tt.type;
							tempType.bindless = tt.bindless;
							tempType.dimension = type;
							tempType.structured = false;
							return true;
						}
					}
				}
				else if (line.substr(0, 6) == "Buffer")
				{
					auto tt = parseTemplateTypeAndName(line, strlen("Buffer"));
					tempType.name = tt.name;
					tempType.type = ResourceType::Buffer;
					tempType.access = ResourceAccess::Read;
					tempType.templatedType = tt.type;
					tempType.bindless = tt.bindless;
					tempType.dimension = "Buffer";
					tempType.structured = isStructured(tempType.dimension);
					return true;
				}
				else if (piece2 == "RW")
				{
					for (auto&& type : WriteBufferTypes)
					{
						if (line.substr(0, strlen(type)) == type)
						{
							auto tt = parseTemplateTypeAndName(line, strlen(type));
							tempType.name = tt.name;
							tempType.type = ResourceType::Buffer;
							tempType.access = ResourceAccess::Write;
							tempType.templatedType = tt.type;
							tempType.bindless = tt.bindless;
							tempType.dimension = type;
							tempType.structured = isStructured(tempType.dimension);
							return true;
						}
					}
				}
				else
				{
					if (line.substr(0, strlen("AppendStructuredBuffer")) == "AppendStructuredBuffer")
					{
						auto tt = parseTemplateTypeAndName(line, strlen("AppendStructuredBuffer"));
						tempType.name = tt.name;
						tempType.type = ResourceType::Buffer;
						tempType.access = ResourceAccess::Write;
						tempType.templatedType = tt.type;
						tempType.bindless = tt.bindless;
						tempType.structured = true;
						tempType.dimension = "AppendStructuredBuffer";
						return true;
					}
					else if (line.substr(0, strlen("StructuredBuffer")) == "StructuredBuffer")
					{
						auto tt = parseTemplateTypeAndName(line, strlen("StructuredBuffer"));
						tempType.name = tt.name;
						tempType.type = ResourceType::Buffer;
						tempType.access = ResourceAccess::Read;
						tempType.templatedType = tt.type;
						tempType.bindless = tt.bindless;
						tempType.structured = true;
						tempType.dimension = "StructuredBuffer";
						return true;
					}
					else if (line.substr(0, strlen("sampler")) == "sampler")
					{
						auto from = strlen("sampler");
						// jump over empty space
						while (line[from] == ' ')
							++from;

						auto nameStart = from;
						// jump forward
						while (line[from] != ';')
							++from;
						auto nameStop = from;

						tempType.type = ResourceType::Sampler;
						tempType.name = line.substr(nameStart, nameStop - nameStart);
						tempType.structured = false;
						tempType.dimension = "sampler";
						return true;
					}
					else if (line.substr(0, strlen("SamplerComparisonState")) == "SamplerComparisonState")
					{
						auto from = strlen("SamplerComparisonState");
						// jump over empty space
						while (line[from] == ' ')
							++from;

						auto nameStart = from;
						// jump forward
						while (line[from] != ';')
							++from;
						auto nameStop = from;

						tempType.type = ResourceType::Sampler;
						tempType.name = line.substr(nameStart, nameStop - nameStart);
						tempType.structured = false;
						tempType.dimension = "sampler";
						return true;
					}
					else if (line.substr(0, strlen("ByteAddressBuffer")) == "ByteAddressBuffer")
					{
						auto from = strlen("ByteAddressBuffer");
						// jump over empty space
						while (line[from] == ' ')
							++from;

						auto nameStart = from;
						// jump forward
						while (line[from] != ';')
							++from;
						auto nameStop = from;

						tempType.type = ResourceType::Buffer;
						tempType.name = line.substr(nameStart, nameStop - nameStart);
						tempType.dimension = "ByteAddressBuffer";
						tempType.structured = false;
						return true;
					}
					else if (line.substr(0, strlen("ConstantBuffer")) == "ConstantBuffer")
					{
						auto tt = parseTemplateTypeAndName(line, strlen("ConstantBuffer"));
						tempType.name = tt.name;
						tempType.type = ResourceType::RootConstant;
						tempType.access = ResourceAccess::Read;
						tempType.templatedType = tt.type;
						tempType.bindless = tt.bindless;
						tempType.dimension = "ConstantBuffer";
						tempType.structured = false;
						return true;
					}
					else if (line.substr(0, strlen("RaytracingAccelerationStructure")) == "RaytracingAccelerationStructure")
					{
						auto from = strlen("RaytracingAccelerationStructure");
						// jump over empty space
						while (line[from] == ' ')
							++from;

						auto nameStart = from;
						// jump forward
						while ((line[from] != ';') && (line[from] != '{'))
							++from;
						auto nameStop = from;

						tempType.type = ResourceType::AccelerationStructure;
						tempType.name = line.substr(nameStart, nameStop - nameStart);
						tempType.dimension = "RaytracingAccelerationStructure";
						tempType.structured = false;
						return true;
					}
				}
			}
			return false;
		};

		auto inCompleteSystemTypes = [&](const engine::string& type)->bool
		{
			auto t1 = type[0];
			if (t1 == 'v')
				return type == "void";
			if (t1 == 'b')
				return type == "bool";
			if (t1 == 'h')
				return type == "half";

			if (t1 == 'i')
			{
				for (int i = 3; i < 24; ++i)
					if (CompleteSystemTypes[i] == type)
						return true;
				return false;
			}

			if (t1 == 'u')
			{
				for (int i = 24; i < 45; ++i)
					if (CompleteSystemTypes[i] == type)
						return true;
				return false;
			}

			if (t1 == 'f')
			{
				for (int i = 45; i < 66; ++i)
					if (CompleteSystemTypes[i] == type)
						return true;
				return false;
			}

			if (t1 == 'd')
			{
				for (int i = 66; i < 88; ++i)
					if (CompleteSystemTypes[i] == type)
						return true;
				return false;
			}

			if (t1 == 't')
				return type == "texture";
			if (t1 == 's')
				return type == "sampler";
			if (t1 == 'S')
				return type == "SamplerComparisonState";
			if (t1 == 'R')
				return type == "RaytracingAccelerationStructure";
			if (t1 == 'B')
				return type == "ByteAddressBuffer";

			return false;
			/*for (auto&& t : CompleteSystemTypes)
				if (t == type)
					return true;
			return false;*/
		};

		auto inAllTemplatedSystemTypes = [&](const engine::string& type)->bool
		{
			for (auto&& t : AllTemplatedSystemTypes)
				if (t == type)
					return true;
			return false;
		};

		auto removeIgnorable = [](int& enter, int& index, engine::vector<engine::string>& lines)
		{
			bool insideComment = false;

			while (index < lines.size())
			{
				auto c = lines[index][enter];
				if ((!insideComment && std::isalpha(c)) ||
					(!insideComment && c == '}'))
					return;

				if (c == '/')
				{
					if (enter + 1 < lines[index].size() && lines[index][enter + 1] == '/')
					{
						// commented the rest of the line
						enter = 0;
						++index;
					}
					else if (enter + 1 < lines[index].size() && lines[index][enter + 1] == '*')
					{
						++enter;
						insideComment = true;
					}
				}
				else if (c == '*')
				{
					if (enter + 1 < lines[index].size() && lines[index][enter + 1] == '/')
					{
						// commented the rest of the line
						++enter;
						insideComment = false;
					}
				}
				++enter;
				if (enter >= lines[index].size())
				{
					enter = 0;
					++index;
				}
			}
		};

		//for (auto&& line : lines)
		for(int i = 0; i < lines.size(); ++i)
		{
			ResourceMapping ttype = {};

			if (inTemplatedSystemTypes(lines[i], ttype))
			{
				ttype.lineNumber = i;
				if (!ttype.bindless)
				{
					if (ttype.type == ResourceType::Texture)
					{
						if (ttype.access == ResourceAccess::Read)
						{
							m_textureSRV.emplace_back(ttype);
						}
						else if (ttype.access == ResourceAccess::Write)
						{
							m_textureUAV.emplace_back(ttype);
						}
					}
					else if (ttype.type == ResourceType::Buffer)
					{
						if (ttype.access == ResourceAccess::Read)
						{
							m_bufferSRV.emplace_back(ttype);
						}
						else if (ttype.access == ResourceAccess::Write)
						{
							m_bufferUAV.emplace_back(ttype);
						}
					}
					else if (ttype.type == ResourceType::Sampler)
					{
						m_samplers.emplace_back(ttype);
					}
					else if (ttype.type == ResourceType::RootConstant)
					{
						m_rootConstants.emplace_back(ttype);
					}
					else if (ttype.type == ResourceType::AccelerationStructure)
					{
						m_accelerationStructures.emplace_back(ttype);
					}
				}
				else
				{
					if (ttype.type == ResourceType::Texture)
					{
						if (ttype.access == ResourceAccess::Read)
						{
							m_bindlessTextureSRV.emplace_back(ttype);
						}
						else if (ttype.access == ResourceAccess::Write)
						{
							m_bindlessTextureUAV.emplace_back(ttype);
						}
					}
					else if (ttype.type == ResourceType::Buffer)
					{
						if (ttype.access == ResourceAccess::Read)
						{
							m_bindlessBufferSRV.emplace_back(ttype);
						}
						else if (ttype.access == ResourceAccess::Write)
						{
							m_bindlessBufferUAV.emplace_back(ttype);
						}
					}
				}
			}
			else if (lines[i].substr(0, 7) == "cbuffer")
			{
				int nameStart = 8;
				int nameStop = nameStart;
				while ((nameStop < lines[i].length()) && (std::isalnum(lines[i][nameStop]) || lines[i][nameStop] == '_'))
					++nameStop;

				ResourceMapping res;
				res.name = lines[i].substr(nameStart, nameStop - nameStart);
				res.type = ResourceType::Buffer;
				res.dimension = "cbuffer";
				res.lineNumber = i;

				auto enter = lines[i].find("{", nameStop);
				while(enter == engine::string::npos)
				{
					nameStop = 0;
					++i;
					enter = lines[i].find("{", nameStop);
				}
				int currentPos = enter+1;

				char currentChar = lines[i][currentPos];
				while (currentChar != '}')
				{
					removeIgnorable(currentPos, i, lines);

					auto rest = lines[i].substr(currentPos, lines[i].length() - currentPos);
					auto split = engine::tokenize(rest, { ' ' });
					ASSERT(split.size() >= 2, "variable name on the wrong line in constant buffer definition. line: %i", i);

					while (split[1][split[1].length() - 1] == '\n')
						split[1] = split[1].substr(0, split[1].length() - 1);
					while (split[1][split[1].length() - 1] == ';')
						split[1] = split[1].substr(0, split[1].length() - 1);

					res.constants.emplace_back(std::make_pair( split[0], split[1] ));
					++i;
					currentPos = 0;

					currentChar = lines[i][currentPos];
				}
				m_constants.emplace_back(res);
			}
			else if (lines[i].find("main(") != engine::string::npos)
			{
				// need to parse main function parameters for shader input parameters
				//auto parts = engine::tokenize(lines[i], { ' ', ':', ',', '(', ')', '\n' });
				auto parts = engine::tokenize(lines[i], { '(', ')' });
				auto params = engine::tokenize(parts[1], { ',' });

				//ResourceMapping res;
				//res.name = "main";
				//res.type = ResourceType::Function;
				//res.dimension = "main function";
				//res.lineNumber = i;

				auto funcInfo = engine::tokenize(parts[0], { ' ' });
				auto returnType = funcInfo[0];
				auto functionName = funcInfo[1];

				for (auto&& p : params)
				{
					auto tns = engine::tokenize(p, {' ', ':'});
					if (tns.size() == 2)
					{
						bool handled = false;
						for(auto&& knownStruct : m_knownStructures)
							if (knownStruct.name == tns[0])
							{
								m_inputParameters.reserve(m_inputParameters.size() + distance(knownStruct.param.begin(), knownStruct.param.end()));
								m_inputParameters.insert(m_inputParameters.end(), knownStruct.param.begin(), knownStruct.param.end());
								handled = true;
								break;
							}
						ASSERT(handled, "Did not find an input parameter struct");
						
					}
					else if (tns.size() == 3)
						m_inputParameters.emplace_back(InputParameter{ tns[1], tns[0], tns[2] });
				}

				//for (int i = 2; i < parts.size();)
				//{
				//	if (inAllTemplatedSystemTypes(parts[i]) || inCompleteSystemTypes(parts[i]))
				//	{
				//		ASSERT(i + 2 < parts.size(), "Non complete type in shader main function parameter list");
				//		res.inputParameters.emplace_back(InputParameter{ parts[i], parts[i + 1], parts[i + 2] });
				//		i += 3;
				//	}
				//	else
				//	{
				//		res.inputParameters.emplace_back(InputParameter{ parts[i], parts[i + 1], "" });
				//		i += 2;
				//	}
				//}
			}
			else
			{
				// check for structures
				auto parts = engine::tokenize(lines[i], { ' ', '\n' });
				bool structureDef = false;
				int startIndex = 0;
				int pindex = 0;
				for (auto&& p : parts)
					if (p == "struct")
					{
						structureDef = true;
						break;
					}
					else
					{
						startIndex += p.length();
						++pindex;
					}

				if (structureDef)
				{
					auto structName = parts[pindex + 1];

					m_knownStructures.emplace_back(StructDef{structName});
					StructDef& newStruct = m_knownStructures.back();

					auto curLineIndex = i;
					auto chrIndex = lines[curLineIndex].find(structName) + structName.length();

					auto next = [&]()
					{
						++chrIndex;
						if (chrIndex >= lines[curLineIndex].size())
						{
							chrIndex = 0;
							++curLineIndex;
						}
					};

					auto readEmptyAway = [&]()
					{
						while ((lines[curLineIndex][chrIndex] == ' ') ||
							(lines[curLineIndex][chrIndex] == '\t') ||
							(lines[curLineIndex][chrIndex] == '\n')) {
							next();
						}
					};

					// read until structure def starts
					while (lines[curLineIndex][chrIndex] != '{') { next(); }
					next();

					// read all variables
					while (lines[curLineIndex][chrIndex] != '}')
					{

						readEmptyAway();

						// read type
						engine::string typeString;
						while (std::isalnum(lines[curLineIndex][chrIndex]) || lines[curLineIndex][chrIndex] == '_')
						{
							typeString += lines[curLineIndex][chrIndex];
							next();
						}

						readEmptyAway();

						// read type name
						engine::string typeNameString;
						while (std::isalnum(lines[curLineIndex][chrIndex]))
						{
							typeNameString += lines[curLineIndex][chrIndex];
							next();
						}

						readEmptyAway();

						bool readSemantic = false;
						if (lines[curLineIndex][chrIndex] == ':')
						{
							readSemantic = true;
							next();
						}
						else if (lines[curLineIndex][chrIndex] == ';')
						{
							next();
						}
						else
						{
							LOG_PURE("missing semicolon");
							next();
						}

						engine::string typeSemanticString;
						if (readSemantic)
						{
							readEmptyAway();

							// read semantic
							while (std::isalnum(lines[curLineIndex][chrIndex]) || lines[curLineIndex][chrIndex] == '_')
							{
								typeSemanticString += lines[curLineIndex][chrIndex];
								next();
							}

							readEmptyAway();

							if (lines[curLineIndex][chrIndex] == ';')
								next();
							else
							{
								LOG_PURE("missing semicolon");
								next();
							}
						}
						newStruct.param.emplace_back(InputParameter{ typeNameString, typeString, typeSemanticString });

						readEmptyAway();
					}
				}
			}
		}
    }
}