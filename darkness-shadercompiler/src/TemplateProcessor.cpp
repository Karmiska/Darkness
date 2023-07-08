#include "TemplateProcessor.h"
#include "inja.hpp"
#include "containers/unordered_map.h"
#include "tools/Debug.h"
#include "tools/PathTools.h"
#include "ShaderPathTools.h"
#include "ShaderLocator.h"
#include "Helpers.h"

namespace shadercompiler
{
	
	void to_json(inja::json& j, const PermutationItem& p)
	{
		auto temp = p.variableName;
		std::transform(
			temp.begin(),
			temp.begin() + 1,
			temp.begin(), ::tolower);

		j["type"] = p.type;
		j["variableName"] = temp;
		j["value"] = p.value;
		j["flag"] = p.flag;
	}
	void from_json(const inja::json& j, PermutationItem& p)
	{
		j.at("type").get_to(p.type);
		j.at("variableName").get_to(p.variableName);
		j.at("value").get_to(p.value);
		j.at("flag").get_to(p.flag);

		std::transform(
			p.variableName.begin(),
			p.variableName.begin() + 1,
			p.variableName.begin(), ::toupper);

	}

	void to_json(inja::json& j, const Permutation& p)
	{
		j["list"] = p.list;
		j["id"] = p.id;
		j["defines"] = p.defines;
	}
	void from_json(const inja::json& j, Permutation& p)
	{
		j.at("list").get_to(p.list);
		j.at("id").get_to(p.id);
		j.at("defines").get_to(p.defines);
	}


	struct ConstantIdentifier
	{
		engine::string name;
		engine::string type;
	};

	struct ConstantItem
	{
		engine::string name;
		engine::vector<ConstantIdentifier> identifiers;
	};

	void to_json(inja::json& j, const ConstantIdentifier& p)
	{
		j["name"] = p.name;
		j["type"] = p.type;
	}
	void from_json(const inja::json& j, ConstantIdentifier& p)
	{
		j.at("name").get_to(p.name);
		j.at("type").get_to(p.type);
	}

	void to_json(inja::json& j, const ConstantItem& p)
	{
		j["name"] = p.name;
		j["identifiers"] = p.identifiers;
	}
	void from_json(const inja::json& j, ConstantItem& p)
	{
		j.at("name").get_to(p.name);
		j.at("identifiers").get_to(p.identifiers);
	}


	struct EnumItem
	{
		engine::string name;
		engine::string nameLower;
		int length;
		engine::vector<engine::string> values;
	};

	void to_json(inja::json& j, const EnumItem& p)
	{
		j["name"] = p.name;
		j["name_lower"] = p.nameLower;
		j["length"] = p.length;
		j["values"] = p.values;
	}
	void from_json(const inja::json& j, EnumItem& p)
	{
		j.at("name").get_to(p.name);
		j.at("name_lower").get_to(p.nameLower);
		j.at("length").get_to(p.length);
		j.at("values").get_to(p.values);
	}


	using ConstantStructures = engine::vector<ConstantItem>;
	using ResourceStructures = engine::vector<engine::unordered_map<engine::string, engine::string>>;
	using ParameterPermutationStructures = engine::vector<ResourceStructures>;
	using EnumStructures = engine::vector<EnumItem>;

    struct TemplateData
    {
		bool has_constants;
		bool has_texture_srvs;
		bool has_texture_uavs;
		bool has_bindless_texture_srvs;
		bool has_bindless_texture_uavs;
		bool has_buffer_srvs;
		bool has_buffer_uavs;
		bool has_acceleration_structures;
		bool has_bindless_buffer_srvs;
		bool has_bindless_buffer_uavs;
		bool has_samplers;
		bool has_root_constants;
		bool has_debug_output;
		engine::string ShaderClass;
		engine::string ShaderClassLower;
		engine::string class_type;
		engine::string ShaderLoadInterfaceHeader;
		engine::string shader_pipeline_configuration_class;
		engine::string ShaderBinaryPath;
		engine::string BaseExt;
		engine::string BasePathAndFile;
		engine::string ShaderSupportPath;
		int shader_pipeline_permutation_count;
		
		ConstantStructures constant_structures;
		ResourceStructures texture_srvs;
		ResourceStructures texture_uavs;
		ResourceStructures bindless_texture_srvs;
		ResourceStructures bindless_texture_uavs;
		ResourceStructures buffer_srvs;
		ResourceStructures buffer_uavs;
		ResourceStructures acceleration_structures;
		ResourceStructures bindless_buffer_srvs;
		ResourceStructures bindless_buffer_uavs;
		ResourceStructures samplers;
		ResourceStructures root_constants;
		ResourceStructures options;
		EnumStructures enums;
		engine::vector<Permutation> permutations;

		int descriptor_count;

		ResourceStructures srvs;
		ResourceStructures uavs;
		ResourceStructures dimensions;

		ParameterPermutationStructures input_parameters;

		ResourceStructures srvs_bindings;
		ResourceStructures uavs_bindings;
		ResourceStructures acceleration_bindings;

		engine::string set_start_index;
		engine::string set_count;
    };

	void to_json(inja::json& j, const TemplateData& p)
	{
		j["has_constants"] = p.has_constants;
		j["has_texture_srvs"] = p.has_texture_srvs;
		j["has_texture_uavs"] = p.has_texture_uavs;
		j["has_bindless_texture_srvs"] = p.has_bindless_texture_srvs;
		j["has_bindless_texture_uavs"] = p.has_bindless_texture_uavs;
		j["has_buffer_srvs"] = p.has_buffer_srvs;
		j["has_buffer_uavs"] = p.has_buffer_uavs;
		j["has_acceleration_structures"] = p.has_acceleration_structures;
		j["has_bindless_buffer_srvs"] = p.has_bindless_buffer_srvs;
		j["has_bindless_buffer_uavs"] = p.has_bindless_buffer_uavs;
		j["has_samplers"] = p.has_samplers;
		j["has_root_constants"] = p.has_root_constants;
		j["has_debug_output"] = p.has_debug_output;
		j["ShaderClass"] = p.ShaderClass;
		j["ShaderClassLower"] = p.ShaderClassLower;
		j["class_type"] = p.class_type;
		j["ShaderLoadInterfaceHeader"] = p.ShaderLoadInterfaceHeader;
		j["shader_pipeline_configuration_class"] = p.shader_pipeline_configuration_class;
		j["ShaderBinaryPath"] = p.ShaderBinaryPath;
		j["BaseExt"] = p.BaseExt;
		j["BasePathAndFile"] = p.BasePathAndFile;
		j["ShaderSupportPath"] = p.ShaderSupportPath;
		j["shader_pipeline_permutation_count"] = p.shader_pipeline_permutation_count;

		j["constant_structures"] = p.constant_structures;
		j["texture_srvs"] = p.texture_srvs;
		j["texture_uavs"] = p.texture_uavs;
		j["bindless_texture_srvs"] = p.bindless_texture_srvs;
		j["bindless_texture_uavs"] = p.bindless_texture_uavs;
		j["buffer_srvs"] = p.buffer_srvs;
		j["buffer_uavs"] = p.buffer_uavs;
		j["acceleration_structures"] = p.acceleration_structures;
		j["bindless_buffer_srvs"] = p.bindless_buffer_srvs;
		j["bindless_buffer_uavs"] = p.bindless_buffer_uavs;
		j["samplers"] = p.samplers;
		j["root_constants"] = p.root_constants;
		j["options"] = p.options;
		j["enums"] = p.enums;
		j["permutations"] = p.permutations;
		j["descriptor_count"] = p.descriptor_count;

		j["srvs"] = p.srvs;
		j["uavs"] = p.uavs;
		j["dimensions"] = p.dimensions;

		j["input_parameters"] = p.input_parameters;

		j["srvs_bindings"] = p.srvs_bindings;
		j["uavs_bindings"] = p.uavs_bindings;
		j["acceleration_bindings"] = p.acceleration_bindings;

		j["set_start_index"] = p.set_start_index;
		j["set_count"] = p.set_count;
	}

	void from_json(const inja::json& j, TemplateData& p)
	{
		j.at("has_constants").get_to(p.has_constants);
		j.at("has_texture_srvs").get_to(p.has_texture_srvs);
		j.at("has_texture_uavs").get_to(p.has_texture_uavs);
		j.at("has_bindless_texture_srvs").get_to(p.has_bindless_texture_srvs);
		j.at("has_bindless_texture_uavs").get_to(p.has_bindless_texture_uavs);
		j.at("has_buffer_srvs").get_to(p.has_buffer_srvs);
		j.at("has_buffer_uavs").get_to(p.has_buffer_uavs);
		j.at("has_acceleration_structures").get_to(p.has_acceleration_structures);
		j.at("has_bindless_buffer_srvs").get_to(p.has_bindless_buffer_srvs);
		j.at("has_bindless_buffer_uavs").get_to(p.has_bindless_buffer_uavs);
		j.at("has_samplers").get_to(p.has_samplers);
		j.at("has_root_constants").get_to(p.has_root_constants);
		j.at("has_debug_output").get_to(p.has_debug_output);
		j.at("ShaderClass").get_to(p.ShaderClass);
		j.at("ShaderClassLower").get_to(p.ShaderClassLower);
		j.at("class_type").get_to(p.class_type);
		j.at("ShaderLoadInterfaceHeader").get_to(p.ShaderLoadInterfaceHeader);
		j.at("shader_pipeline_configuration_class").get_to(p.shader_pipeline_configuration_class);
		j.at("ShaderBinaryPath").get_to(p.ShaderBinaryPath);
		j.at("BaseExt").get_to(p.BaseExt);
		j.at("BasePathAndFile").get_to(p.BasePathAndFile);
		j.at("ShaderSupportPath").get_to(p.ShaderSupportPath);
		j.at("shader_pipeline_permutation_count").get_to(p.shader_pipeline_permutation_count);

		j.at("constant_structures").get_to(p.constant_structures);
		j.at("texture_srvs").get_to(p.texture_srvs);
		j.at("texture_uavs").get_to(p.texture_uavs);
		j.at("bindless_texture_srvs").get_to(p.bindless_texture_srvs);
		j.at("bindless_texture_uavs").get_to(p.bindless_texture_uavs);
		j.at("buffer_srvs").get_to(p.buffer_srvs);
		j.at("buffer_uavs").get_to(p.buffer_uavs);
		j.at("acceleration_structures").get_to(p.acceleration_structures);
		j.at("bindless_buffer_srvs").get_to(p.bindless_buffer_srvs);
		j.at("bindless_buffer_uavs").get_to(p.bindless_buffer_uavs);
		j.at("samplers").get_to(p.samplers);
		j.at("root_constants").get_to(p.root_constants);
		j.at("options").get_to(p.options);
		j.at("enums").get_to(p.enums);
		j.at("permutations").get_to(p.permutations);
		j.at("descriptor_count").get_to(p.descriptor_count);

		j.at("srvs").get_to(p.srvs);
		j.at("uavs").get_to(p.uavs);
		j.at("dimensions").get_to(p.dimensions);

		j.at("input_parameters").get_to(p.input_parameters);

		j.at("srvs_bindings").get_to(p.srvs_bindings);
		j.at("uavs_bindings").get_to(p.uavs_bindings);
		j.at("acceleration_bindings").get_to(p.acceleration_bindings);

		j.at("set_start_index").get_to(p.set_start_index);
		j.at("set_count").get_to(p.set_count);
	}

    void TemplateProcessor::ProcessShaderLoadInterfaces(
		ShaderLocator& locator,
		const engine::string& originalPath,
		const engine::string& templatePath, 
		const engine::string& outputPath, 
		ShaderPipelineStage& stage,
		LogLevel logLevel)
    {
		TemplateData templateData = {};

		auto& resourceMap = *stage.items[0].resources;

		bool debugOutput = false;
		for (auto&& uav : resourceMap.bufferUAV())
		{
			if (uav.name == "debugOutput")
			{
				debugOutput = true;
				break;
			}
		}

		templateData.has_constants = resourceMap.constantBuffers().size() > 0;
		templateData.has_texture_srvs = resourceMap.textureSRV().size() > 0;
		templateData.has_texture_uavs = resourceMap.textureUAV().size() > 0;
		templateData.has_bindless_texture_srvs = resourceMap.bindlessTextureSRV().size() > 0;
		templateData.has_bindless_texture_uavs = resourceMap.bindlessTextureUAV().size() > 0;
		templateData.has_buffer_srvs = resourceMap.bufferSRV().size() > 0;
		templateData.has_buffer_uavs = resourceMap.bufferUAV().size() > 0;
		templateData.has_acceleration_structures = resourceMap.accelerationStructures().size() > 0;
		templateData.has_bindless_buffer_srvs = resourceMap.bindlessBufferSRV().size() > 0;
		templateData.has_bindless_buffer_uavs = resourceMap.bindlessBufferUAV().size() > 0;
		templateData.has_samplers = resourceMap.samplers().size() > 0;
		templateData.has_root_constants = resourceMap.rootConstants().size() > 0;
		templateData.has_debug_output = debugOutput;

		for (auto&& option : stage.options)
		{
			auto val = option.value;
			std::transform(
				val.begin(),
				val.begin() + 1,
				val.begin(), ::tolower);

			engine::unordered_map<engine::string, engine::string> opt;
			opt["value"] = val;
			opt["define"] = option.define;
			templateData.options.emplace_back(opt);
		}

		for (auto&& enumitem : stage.enums)
		{
			EnumItem res;
			res.name = enumitem.first;
			res.nameLower = enumitem.first;
			std::transform(
				res.nameLower.begin(),
				res.nameLower.begin() + 1,
				res.nameLower.begin(), ::tolower);
			res.length = enumitem.second.size();
			for (auto&& val : enumitem.second)
			{
				res.values.emplace_back(val.value);
			}
			templateData.enums.emplace_back(res);
		}

		templateData.permutations = stage.permutations;

		templateData.ShaderClass = className(originalPath);
		templateData.ShaderClassLower = className(originalPath);
		std::transform(templateData.ShaderClassLower.begin(), templateData.ShaderClassLower.end(), templateData.ShaderClassLower.begin(), ::tolower);

		templateData.class_type = stageName(originalPath) + "Shader";
		templateData.ShaderLoadInterfaceHeader = interfacePath(originalPath);
		templateData.shader_pipeline_configuration_class = pipelineNameFromFilename(originalPath);

		auto binFolderRelative = stage.filename.substr(locator.shaderRootPath().length(), stage.filename.length() - locator.shaderRootPath().length());
		auto binaryTarget = engine::pathJoin(locator.shaderBinaryPathDX12(), binFolderRelative);
		templateData.ShaderBinaryPath = engine::pathReplaceExtension(binaryTarget, "cso");
		templateData.ShaderBinaryPath = templateData.ShaderBinaryPath.substr(locator.shaderCorePath().length() + 1, templateData.ShaderBinaryPath.length() - (locator.shaderCorePath().length()+1));
		std::replace(templateData.ShaderBinaryPath.begin(), templateData.ShaderBinaryPath.end(), '\\', '/');

		templateData.BaseExt = ".cso";// engine::pathExtractExtension(binaryTarget);
		templateData.BasePathAndFile = engine::pathJoin(
			engine::pathExtractFolder(binaryTarget),
			engine::pathExtractFilenameWithoutExtension(binaryTarget));
		templateData.BasePathAndFile = templateData.BasePathAndFile.substr(locator.shaderCorePath().length() + 1, templateData.BasePathAndFile.length() - (locator.shaderCorePath().length()+1));
		std::replace(templateData.BasePathAndFile.begin(), templateData.BasePathAndFile.end(), '\\', '/');

		templateData.ShaderSupportPath = engine::pathReplaceExtension(templateData.ShaderBinaryPath, "support");
		templateData.shader_pipeline_permutation_count = stage.items.size();

		for (auto&& constantBuffer : resourceMap.constantBuffers())
		{
			ConstantItem constItem;
			constItem.name = constantBuffer.name;
			for (auto& value : constantBuffer.constants)
			{
				ConstantIdentifier identifier;
				identifier.type = value.first;
				std::transform(
					identifier.type.begin(), 
					identifier.type.begin()+1,
					identifier.type.begin(), ::toupper);
				identifier.name = value.second;
				constItem.identifiers.emplace_back(identifier);
			}
			templateData.constant_structures.emplace_back(constItem);
			++templateData.descriptor_count;
		}

		//using ResourceStructures = engine::vector<engine::unordered_map<engine::string, engine::string>>;
		//using ParameterPermutationStructures = engine::vector<engine::vector<engine::unordered_map<engine::string, engine::string>>>;

		for (auto&& item : stage.items)
		{
			auto& permutationInputParameter = *item.resources.get();
			ResourceStructures permutationParameters;
			for (auto&& inputParameter : permutationInputParameter.inputParameters())
			{
				engine::unordered_map<engine::string, engine::string> opt;
				opt["name"] = inputParameter.name;
				opt["semantic"] = inputParameter.semantic;
				opt["type"] = inputParameter.type;
				permutationParameters.emplace_back(opt);
			}
			templateData.input_parameters.emplace_back(permutationParameters);
		}

		auto getDXFormat = [](const engine::string& format)->engine::string
		{
			if(format == "float")
				return "Format::R32_FLOAT";
			if (format == "float2")
				return "Format::R32G32_FLOAT";
			if (format == "float3")
				return "Format::R32G32B32_FLOAT";
			if (format == "float4")
				return "Format::R32G32B32A32_FLOAT";
			if (format == "uint")
				return "Format::R32_UINT";
			if (format == "uint2")
				return "Format::R32G32_UINT";
			if (format == "uint3")
				return "Format::R32G32B32_UINT";
			if (format == "uint4")
				return "Format::R32G32B32A32_UINT";
			return "Format::UNKNOWN";
		};

		using ResEntry = engine::unordered_map<engine::string, engine::string>;
        for (auto&& res : resourceMap.textureSRV())
        {
			templateData.srvs_bindings.emplace_back(ResEntry{ { "type", "SRVTexture" }, { "index", std::to_string(templateData.texture_srvs.size()) }, { "dimension", res.dimension }, { "format", getDXFormat(res.templatedType) } });
			templateData.texture_srvs.emplace_back(ResEntry{ { "type", "TextureSRV" }, { "identifier", res.name }, { "cube", res.dimension.find("Cube") != engine::string::npos ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.srvs.emplace_back(ResEntry{ { "type", "TextureSRV" }, { "identifier", res.name } });
			templateData.dimensions.emplace_back(ResEntry{ { "type", "TextureSRV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
        }
		for (auto&& res : resourceMap.textureUAV())
		{
			templateData.uavs_bindings.emplace_back(ResEntry{ { "type", "UAVTexture" }, { "index", std::to_string(templateData.texture_uavs.size()) }, { "dimension", res.dimension.substr(2, res.dimension.length()-2) }, { "format", getDXFormat(res.templatedType) } });
			templateData.texture_uavs.emplace_back(ResEntry{ { "type", "TextureUAV" }, { "identifier", res.name }, { "cube", res.dimension.find("Cube") != engine::string::npos ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.uavs.emplace_back(ResEntry{ { "type", "TextureUAV" }, { "identifier", res.name } });
			templateData.dimensions.emplace_back(ResEntry{ { "type", "TextureUAV" }, { "identifier", res.name }, { "dimension", res.dimension.substr(2, res.dimension.length() - 2) } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bindlessTextureSRV())
		{
			templateData.srvs_bindings.emplace_back(ResEntry{ { "type", "BindlessSRVTexture" }, { "index", std::to_string(templateData.bindless_texture_srvs.size()) }, { "dimension", res.dimension }, { "format", getDXFormat(res.templatedType) } });
			templateData.bindless_texture_srvs.emplace_back(ResEntry{ { "type", "BindlessTextureSRV" }, { "identifier", res.name }, { "cube", res.dimension.find("Cube") != engine::string::npos ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.srvs.emplace_back(ResEntry{ { "type", "TextureSRV" }, { "identifier", res.name } });
			templateData.dimensions.emplace_back(ResEntry{ { "type", "TextureSRV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bindlessTextureUAV())
		{
			templateData.uavs_bindings.emplace_back(ResEntry{ { "type", "BindlessUAVTexture" }, { "index", std::to_string(templateData.bindless_texture_uavs.size()) }, { "dimension", res.dimension.substr(2, res.dimension.length() - 2) }, { "format", getDXFormat(res.templatedType) } });
			templateData.bindless_texture_uavs.emplace_back(ResEntry{ { "type", "BindlessTextureUAV" }, { "identifier", res.name }, { "cube", res.dimension.find("Cube") != engine::string::npos ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.uavs.emplace_back(ResEntry{ { "type", "TextureUAV" }, { "identifier", res.name } });
			templateData.dimensions.emplace_back(ResEntry{ { "type", "TextureUAV" }, { "identifier", res.name }, { "dimension", res.dimension.substr(2, res.dimension.length() - 2) } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bufferSRV())
		{
			templateData.srvs_bindings.emplace_back(ResEntry{ { "type", "SRVBuffer" }, { "index", std::to_string(templateData.buffer_srvs.size()) }, { "dimension", "Unknown" }, { "format", getDXFormat(res.templatedType) } });
			templateData.buffer_srvs.emplace_back(ResEntry{ { "type", "BufferSRV" }, { "identifier", res.name }, { "structured", res.structured ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.srvs.emplace_back(ResEntry{ { "type", "BufferSRV" }, { "identifier", res.name } });
			//templateData.dimensions.emplace_back(ResEntry{ { "type", "BufferSRV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bufferUAV())
		{
			templateData.uavs_bindings.emplace_back(ResEntry{ { "type", "UAVBuffer" }, { "index", std::to_string(templateData.buffer_uavs.size()) }, { "dimension", "Unknown" }, { "format", getDXFormat(res.templatedType) } });
			templateData.buffer_uavs.emplace_back(ResEntry{ { "type", "BufferUAV" }, { "identifier", res.name }, { "structured", res.structured ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.uavs.emplace_back(ResEntry{ { "type", "BufferUAV" }, { "identifier", res.name } });
			//templateData.dimensions.emplace_back(ResEntry{ { "type", "BufferUAV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bindlessBufferSRV())
		{
			templateData.srvs_bindings.emplace_back(ResEntry{ { "type", "BindlessSRVBuffer" }, { "index", std::to_string(templateData.bindless_buffer_srvs.size()) }, { "dimension", "Unknown" }, { "format", getDXFormat(res.templatedType) } });
			templateData.bindless_buffer_srvs.emplace_back(ResEntry{ { "type", "BindlessBufferSRV" }, { "identifier", res.name }, { "structured", res.structured ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.srvs.emplace_back(ResEntry{ { "type", "BufferSRV" }, { "identifier", res.name } });
			//templateData.dimensions.emplace_back(ResEntry{ { "type", "BufferSRV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.bindlessBufferUAV())
		{
			templateData.uavs_bindings.emplace_back(ResEntry{ { "type", "BindlessUAVBuffer" }, { "index", std::to_string(templateData.bindless_buffer_uavs.size()) }, { "dimension", "Unknown" }, { "format", getDXFormat(res.templatedType) } });
			templateData.bindless_buffer_uavs.emplace_back(ResEntry{ { "type", "BindlessBufferUAV" }, { "identifier", res.name }, { "structured", res.structured ? "1" : "0" }, { "format", getDXFormat(res.templatedType) } });
			templateData.uavs.emplace_back(ResEntry{ { "type", "BufferUAV" }, { "identifier", res.name } });
			//templateData.dimensions.emplace_back(ResEntry{ { "type", "BufferUAV" }, { "identifier", res.name }, { "dimension", res.dimension } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.accelerationStructures())
		{
			templateData.acceleration_bindings.emplace_back(ResEntry{ { "type", "RaytracingAccelerationStructure" }, { "index", std::to_string(templateData.acceleration_structures.size()) }, { "dimension", "Unknown" }, { "format", "Format::UNKNOWN" } });
			templateData.acceleration_structures.emplace_back(ResEntry{ { "type", "RaytracingAccelerationStructure" }, { "identifier", res.name }, { "structured", res.structured ? "1" : "0" }, { "format", "Format::UNKNOWN" } });
			++templateData.descriptor_count;
		}
		for (auto&& res : resourceMap.samplers())
		{
			templateData.samplers.emplace_back(ResEntry{ { "name", res.name }, { "identifier", res.name } });
			//++templateData.descriptor_count;
		}
		
		for (auto&& rootConstant : resourceMap.rootConstants())
		{
			engine::unordered_map<engine::string, engine::string> opt;
			opt["identifier"] = rootConstant.name;
			opt["type"] = "RootConstant";
			templateData.root_constants.emplace_back(opt);
		}

		templateData.set_start_index = std::to_string(stage.setStart);
		templateData.set_count = std::to_string(stage.setCount());

		inja::json jsondata = templateData;

		engine::vector<char> templateSource;
		{
			std::fstream preprocessedFile;
			preprocessedFile.open(templatePath);
			ASSERT(preprocessedFile.is_open(), "Failed to open template file!");

			preprocessedFile.seekg(0, std::ios::end);
			auto fileSize = static_cast<size_t>(preprocessedFile.tellg());
			preprocessedFile.seekg(0, std::ios::beg);

			templateSource.resize(fileSize);
			preprocessedFile.read(templateSource.data(), fileSize);
			preprocessedFile.close();
		}

		auto result = inja::render(templateSource.data(), jsondata);

		{
			std::fstream outputFile;
			outputFile.open(outputPath + "temporary", std::ios::out);
			outputFile.write(result.c_str(), result.length());
			outputFile.close();
			if(compareAndReplace(outputPath + "temporary", outputPath))
			{
				if (logLevel == shadercompiler::LogLevel::Recompile)
					LOG_PURE("Updated shader load interface for %s", engine::pathExtractFilenameWithoutExtension(outputPath).c_str());
			}
		}
    }

	struct PipelineTemplateData
	{
		bool hasVertexShader;
		bool hasPixelShader;
		bool hasGeometryShader;
		bool hasHullShader;
		bool hasDomainShader;
		bool hasComputeShader;
		bool hasRayGenerationShader;
		bool hasIntersectionShader;
		bool hasMissShader;
		bool hasAnyhitShader;
		bool hasClosestHitShader;
		bool hasAmplificationShader;
		bool hasMeshShader;

		engine::string vertexShaderIf;
		engine::string pixelShaderIf;
		engine::string geometryShaderIf;
		engine::string hullShaderIf;
		engine::string domainShaderIf;
		engine::string computeShaderIf;
		engine::string rayGenerationShaderIf;
		engine::string intersectionShaderIf;
		engine::string missShaderIf;
		engine::string anyHitShaderIf;
		engine::string closestHitShaderIf;
		engine::string amplificationShaderIf;
		engine::string meshShaderIf;

		engine::string vertexShaderType;
		engine::string pixelShaderType;
		engine::string geometryShaderType;
		engine::string hullShaderType;
		engine::string domainShaderType;
		engine::string computeShaderType;
		engine::string rayGenerationShaderType;
		engine::string intersectionShaderType;
		engine::string missShaderType;
		engine::string anyHitShaderType;
		engine::string closestHitShaderType;
		engine::string amplificationShaderType;
		engine::string meshShaderType;

		engine::string pipelineTypeName;
		engine::string pipelineInterfaceFilePath;

	};

	void to_json(inja::json& j, const PipelineTemplateData& p)
	{
		j["has_vertex_shader"] = p.hasVertexShader;
		j["has_pixel_shader"] = p.hasPixelShader;
		j["has_geometry_shader"] = p.hasGeometryShader;
		j["has_hull_shader"] = p.hasHullShader;
		j["has_domain_shader"] = p.hasDomainShader;
		j["has_compute_shader"] = p.hasComputeShader;
		j["has_raygeneration_shader"] = p.hasRayGenerationShader;
		j["has_intersection_shader"] = p.hasIntersectionShader;
		j["has_miss_shader"] = p.hasMissShader;
		j["has_anyhit_shader"] = p.hasAnyhitShader;
		j["has_closesthit_shader"] = p.hasClosestHitShader;
		j["has_amplification_shader"] = p.hasAmplificationShader;
		j["has_mesh_shader"] = p.hasMeshShader;

		j["chas_vertex_shader"] = p.hasVertexShader ? "true" : "false";
		j["chas_pixel_shader"] = p.hasPixelShader ? "true" : "false";
		j["chas_geometry_shader"] = p.hasGeometryShader ? "true" : "false";
		j["chas_hull_shader"] = p.hasHullShader ? "true" : "false";
		j["chas_domain_shader"] = p.hasDomainShader ? "true" : "false";
		j["chas_compute_shader"] = p.hasComputeShader ? "true" : "false";
		j["chas_raygeneration_shader"] = p.hasRayGenerationShader ? "true" : "false";
		j["chas_intersection_shader"] = p.hasIntersectionShader ? "true" : "false";
		j["chas_miss_shader"] = p.hasMissShader ? "true" : "false";
		j["chas_anyhit_shader"] = p.hasAnyhitShader ? "true" : "false";
		j["chas_closesthit_shader"] = p.hasClosestHitShader ? "true" : "false";
		j["chas_amplification_shader"] = p.hasAmplificationShader ? "true" : "false";
		j["chas_mesh_shader"] = p.hasMeshShader ? "true" : "false";

		j["vertex_shader_if"] = p.vertexShaderIf;
		j["pixel_shader_if"] = p.pixelShaderIf;
		j["geometry_shader_if"] = p.geometryShaderIf;
		j["hull_shader_if"] = p.hullShaderIf;
		j["domain_shader_if"] = p.domainShaderIf;
		j["compute_shader_if"] = p.computeShaderIf;
		j["raygeneration_shader_if"] = p.rayGenerationShaderIf;
		j["intersection_shader_if"] = p.intersectionShaderIf;
		j["miss_shader_if"] = p.missShaderIf;
		j["anyhit_shader_if"] = p.anyHitShaderIf;
		j["closesthit_shader_if"] = p.closestHitShaderIf;
		j["amplification_shader_if"] = p.amplificationShaderIf;
		j["mesh_shader_if"] = p.meshShaderIf;

		j["vertex_shader_type"] = p.vertexShaderType;
		j["pixel_shader_type"] = p.pixelShaderType;
		j["geometry_shader_type"] = p.geometryShaderType;
		j["hull_shader_type"] = p.hullShaderType;
		j["domain_shader_type"] = p.domainShaderType;
		j["compute_shader_type"] = p.computeShaderType;
		j["raygeneration_shader_type"] = p.rayGenerationShaderType;
		j["intersection_shader_type"] = p.intersectionShaderType;
		j["miss_shader_type"] = p.missShaderType;
		j["anyhit_shader_type"] = p.anyHitShaderType;
		j["closesthit_shader_type"] = p.closestHitShaderType;
		j["amplification_shader_type"] = p.amplificationShaderType;
		j["mesh_shader_type"] = p.meshShaderType;

		j["pipeline_type_name"] = p.pipelineTypeName;
		j["pipeline_interface_filepath"] = p.pipelineInterfaceFilePath;
	}
	void from_json(const inja::json& j, PipelineTemplateData& p)
	{
		j.at("has_vertex_shader").get_to(p.hasVertexShader);
		j.at("has_pixel_shader").get_to(p.hasPixelShader);
		j.at("has_geometry_shader").get_to(p.hasGeometryShader);
		j.at("has_hull_shader").get_to(p.hasHullShader);
		j.at("has_domain_shader").get_to(p.hasDomainShader);
		j.at("has_compute_shader").get_to(p.hasComputeShader);
		j.at("has_raygeneration_shader").get_to(p.hasRayGenerationShader);
		j.at("has_intersection_shader").get_to(p.hasIntersectionShader);
		j.at("has_miss_shader").get_to(p.hasMissShader);
		j.at("has_anyhit_shader").get_to(p.hasAnyhitShader);
		j.at("has_closesthit_shader").get_to(p.hasClosestHitShader);
		j.at("has_amplification_shader").get_to(p.hasAmplificationShader);
		j.at("has_mesh_shader").get_to(p.hasMeshShader);

		j.at("vertex_shader_if").get_to(p.vertexShaderIf);
		j.at("pixel_shader_if").get_to(p.pixelShaderIf);
		j.at("geometry_shader_if").get_to(p.geometryShaderIf);
		j.at("hull_shader_if").get_to(p.hullShaderIf);
		j.at("domain_shader_if").get_to(p.domainShaderIf);
		j.at("compute_shader_if").get_to(p.computeShaderIf);
		j.at("raygeneration_shader_if").get_to(p.rayGenerationShaderIf);
		j.at("intersection_shader_if").get_to(p.intersectionShaderIf);
		j.at("miss_shader_if").get_to(p.missShaderIf);
		j.at("anyhit_shader_if").get_to(p.anyHitShaderIf);
		j.at("closesthit_shader_if").get_to(p.closestHitShaderIf);
		j.at("amplification_shader_if").get_to(p.amplificationShaderIf);
		j.at("mesh_shader_if").get_to(p.meshShaderIf);

		j.at("vertex_shader_type").get_to(p.vertexShaderType);
		j.at("pixel_shader_type").get_to(p.pixelShaderType);
		j.at("geometry_shader_type").get_to(p.geometryShaderType);
		j.at("hull_shader_type").get_to(p.hullShaderType);
		j.at("domain_shader_type").get_to(p.domainShaderType);
		j.at("compute_shader_type").get_to(p.computeShaderType);
		j.at("raygeneration_shader_type").get_to(p.rayGenerationShaderType);
		j.at("intersection_shader_type").get_to(p.intersectionShaderType);
		j.at("miss_shader_type").get_to(p.missShaderType);
		j.at("anyhit_shader_type").get_to(p.anyHitShaderType);
		j.at("closesthit_shader_type").get_to(p.closestHitShaderType);
		j.at("amplification_shader_type").get_to(p.amplificationShaderType);
		j.at("mesh_shader_type").get_to(p.meshShaderType);

		j.at("pipeline_type_name").get_to(p.pipelineTypeName);
		j.at("pipeline_interface_filepath").get_to(p.pipelineInterfaceFilePath);
	}

	void TemplateProcessor::ProcessPipelineInterfaces(
		const engine::string& templatePath,
		const ShaderPipelineConfiguration& pipeline,
		const engine::string& targetPath,
		LogLevel logLevel)
	{
		auto hasStage = [](const engine::vector<ShaderPipelineStage>& stages, const engine::string& stageName)->bool
		{
			for (auto&& stage : stages)
				if (stage.name == stageName)
					return true;
			return false;
		};

		auto stageInterfaceName = [](const engine::vector<ShaderPipelineStage>& stages, const engine::string& stageName)->engine::string
		{
			for (auto&& stage : stages)
				if (stage.name == stageName)
					return engine::pathExtractFilename(engine::pathReplaceExtension(stage.filename, "h"));
			return "";
		};

		auto shaderTypeName = [](const engine::vector<ShaderPipelineStage>& stages, const engine::string& stageName)->engine::string
		{
			for (auto&& stage : stages)
				if (stage.name == stageName)
					return className(stage.filename);
			return "";
		};

		PipelineTemplateData templateData = {};

		templateData.hasVertexShader = hasStage(pipeline.stages, "Vertex");
		templateData.hasPixelShader = hasStage(pipeline.stages, "Pixel");
		templateData.hasGeometryShader = hasStage(pipeline.stages, "Geometry");
		templateData.hasHullShader = hasStage(pipeline.stages, "Hull");
		templateData.hasDomainShader = hasStage(pipeline.stages, "Domain");
		templateData.hasComputeShader = hasStage(pipeline.stages, "Compute");
		templateData.hasRayGenerationShader = hasStage(pipeline.stages, "Raygeneration");
		templateData.hasIntersectionShader = hasStage(pipeline.stages, "Intersection");
		templateData.hasMissShader = hasStage(pipeline.stages, "Miss");
		templateData.hasAnyhitShader = hasStage(pipeline.stages, "AnyHit");
		templateData.hasClosestHitShader = hasStage(pipeline.stages, "ClosestHit");
		templateData.hasAmplificationShader = hasStage(pipeline.stages, "Amplification");
		templateData.hasMeshShader = hasStage(pipeline.stages, "Mesh");

		templateData.vertexShaderIf = stageInterfaceName(pipeline.stages, "Vertex");
		templateData.pixelShaderIf = stageInterfaceName(pipeline.stages, "Pixel");
		templateData.geometryShaderIf = stageInterfaceName(pipeline.stages, "Geometry");
		templateData.hullShaderIf = stageInterfaceName(pipeline.stages, "Hull");
		templateData.domainShaderIf = stageInterfaceName(pipeline.stages, "Domain");
		templateData.computeShaderIf = stageInterfaceName(pipeline.stages, "Compute");
		templateData.rayGenerationShaderIf = stageInterfaceName(pipeline.stages, "Raygeneration");
		templateData.intersectionShaderIf = stageInterfaceName(pipeline.stages, "Intersection");
		templateData.missShaderIf = stageInterfaceName(pipeline.stages, "Miss");
		templateData.anyHitShaderIf = stageInterfaceName(pipeline.stages, "AnyHit");
		templateData.closestHitShaderIf = stageInterfaceName(pipeline.stages, "ClosestHit");
		templateData.amplificationShaderIf = stageInterfaceName(pipeline.stages, "Amplification");
		templateData.meshShaderIf = stageInterfaceName(pipeline.stages, "Mesh");

		templateData.vertexShaderType = shaderTypeName(pipeline.stages, "Vertex");
		templateData.pixelShaderType = shaderTypeName(pipeline.stages, "Pixel");
		templateData.geometryShaderType = shaderTypeName(pipeline.stages, "Geometry");
		templateData.hullShaderType = shaderTypeName(pipeline.stages, "Hull");
		templateData.domainShaderType = shaderTypeName(pipeline.stages, "Domain");
		templateData.computeShaderType = shaderTypeName(pipeline.stages, "Compute");
		templateData.rayGenerationShaderType = shaderTypeName(pipeline.stages, "Raygeneration");
		templateData.intersectionShaderType = shaderTypeName(pipeline.stages, "Intersection");
		templateData.missShaderType = shaderTypeName(pipeline.stages, "Miss");
		templateData.anyHitShaderType = shaderTypeName(pipeline.stages, "AnyHit");
		templateData.closestHitShaderType = shaderTypeName(pipeline.stages, "ClosestHit");
		templateData.amplificationShaderType = shaderTypeName(pipeline.stages, "Amplification");
		templateData.meshShaderType = shaderTypeName(pipeline.stages, "Mesh");

		templateData.pipelineTypeName = pipeline.pipelineName;
		templateData.pipelineInterfaceFilePath = engine::pathExtractFilename(engine::pathReplaceExtension(targetPath, "h"));

		inja::json jsondata = templateData;

		engine::vector<char> templateSource;
		{
			std::fstream preprocessedFile;
			preprocessedFile.open(templatePath);
			ASSERT(preprocessedFile.is_open(), "Failed to open template file!");

			preprocessedFile.seekg(0, std::ios::end);
			auto fileSize = static_cast<size_t>(preprocessedFile.tellg());
			preprocessedFile.seekg(0, std::ios::beg);

			templateSource.resize(fileSize);
			preprocessedFile.read(templateSource.data(), fileSize);
			preprocessedFile.close();
		}

		auto result = inja::render(templateSource.data(), jsondata);

		{
			std::fstream outputFile;
			outputFile.open(targetPath + "temporary", std::ios::out);
			outputFile.write(result.c_str(), result.length());
			outputFile.close();
			if(compareAndReplace(targetPath + "temporary", targetPath))
			{
				if (logLevel == shadercompiler::LogLevel::Recompile)
					LOG_PURE("Updated pipeline interface for %s", engine::pathExtractFilenameWithoutExtension(targetPath).c_str());
			}
		}
	}
}