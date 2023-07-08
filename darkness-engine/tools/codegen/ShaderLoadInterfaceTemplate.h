#pragma once

#include "tools/hash/SpookyHashV2.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/CommonNoDep.h"
#include "shaders/ShaderTypes.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "containers/string.h"

namespace engine
{
    class ShaderBinary;
    class ShaderStorage;
    class Device;
    class Sampler;

    namespace implementation
    {
        class PipelineImpl;
        class CommandListImpl;
    }

    namespace shaders
    {
        class {{shader_pipeline_configuration_class}};

        {% if has_constants -%}
        namespace {{ShaderClassLower}}_namespace
        {
        {% for item in constant_structures -%}
                struct {{item.name}}
            {
                {% for structure_identifier in item.identifiers -%}
                {{structure_identifier.type}} {{structure_identifier.name}};
                {% endfor -%}
};
        {% endfor -%}
}
        {% endif -%}

        class {{ShaderClass}} : public {{class_type}}
            {% if has_constants -%}
            {% for item in constant_structures -%}
            , public {{ShaderClassLower}}_namespace::{{item.name}}
            {% endfor -%}{% endif -%}
        {
        public:

            {{ShaderClass}}();

            {{ShaderClass}}(const {{ShaderClass}}&);
            {{ShaderClass}}({{ShaderClass}}&&);
            {{ShaderClass}}& operator=(const {{ShaderClass}}&);
            {{ShaderClass}}& operator=({{ShaderClass}}&&);

            {% for item in texture_srvs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}
            {% for item in texture_uavs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}

            {% for item in bindless_texture_srvs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}
            {% for item in bindless_texture_uavs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}

            {% for item in buffer_srvs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}
            {% for item in buffer_uavs -%}
            {{item.type}} {{item.identifier}};
            {% endfor %}
            {% for item in acceleration_structures -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}

            {% for item in bindless_buffer_srvs -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}
            {% for item in bindless_buffer_uavs -%}
            {{item.type}} {{item.identifier}};
            {% endfor %}

            {% for item in samplers -%}
            Sampler {{item.identifier}};
            {% endfor -%}

			{% for item in root_constants -%}
            {{item.type}} {{item.identifier}};
            {% endfor -%}

            {% for item in options -%}
            bool {{item.value}} = false;
            {% endfor -%}

            {% for item in enums -%}
            enum class {{item.name}}
            {
                {% for enumvalue in item.values -%}
                {{enumvalue}},
                {% endfor -%}
            };
            {{item.name}} {{item.name_lower}};
            static constexpr unsigned int {{item.name}}Count = {{item.length}};
            {% endfor -%}

        protected:
            friend class implementation::CommandListImpl;

            engine::ResourceDimension textureDimension(const engine::string& name) const final;
			size_t samplerCount() const final;

			engine::string shaderFilePath() const final;

            void setDebugBuffer(BufferUAV debugOutputBuffer) override;
            bool hasDebugBuffer() const override;
            bool debugBufferIsSet() const override;
        private:
            friend class implementation::PipelineImplDX12;
            friend class implementation::PipelineImplVulkan;
            friend class implementation::PipelineShadersDX12;
            friend class implementation::PipelineShadersVulkan;

            int currentPermutationId() const final;
            engine::shared_ptr<const ShaderBinary> load(const Device& device, ShaderStorage& storage, GraphicsApi api) const final;
            const engine::vector<TextureSRV>& texture_srvs() const final;
            const engine::vector<TextureUAV>& texture_uavs() const final;

            engine::vector<TextureSRV>& texture_srvs() final;
            engine::vector<TextureUAV>& texture_uavs() final;

            const engine::vector<unsigned char>& texture_srvs_is_cube() const final;
            const engine::vector<unsigned char>& texture_uavs_is_cube() const final;

            const engine::vector<Format>& texture_srvs_format() const final;
            const engine::vector<Format>& texture_uavs_format() const final;

            const engine::vector<BufferSRV>& buffer_srvs() const final;
            const engine::vector<BufferUAV>& buffer_uavs() const final;
            engine::vector<BufferSRV>& buffer_srvs() final;
            engine::vector<BufferUAV>& buffer_uavs() final;
            const engine::vector<RaytracingAccelerationStructure>& acceleration_structures() const final;

			const engine::vector<unsigned char>& buffer_srvs_is_structured() const final;
            const engine::vector<unsigned char>& buffer_uavs_is_structured() const final;

            const engine::vector<Format>& buffer_srvs_format() const final;
            const engine::vector<Format>& buffer_uavs_format() const final;
            
            const engine::vector<const BindlessTextureSRV*>& bindless_texture_srvs() const final;
            const engine::vector<const BindlessTextureUAV*>& bindless_texture_uavs() const final;
            const engine::vector<const BindlessBufferSRV*>& bindless_buffer_srvs() const final;
            const engine::vector<const BindlessBufferUAV*>& bindless_buffer_uavs() const final;

            engine::vector<const BindlessTextureSRV*>& bindless_texture_srvs() final;
            engine::vector<const BindlessTextureUAV*>& bindless_texture_uavs() final;
            engine::vector<const BindlessBufferSRV*>& bindless_buffer_srvs() final;
            engine::vector<const BindlessBufferUAV*>& bindless_buffer_uavs() final;
            
            engine::vector<Shader::ConstantRange>& constants() final;
			const engine::vector<Sampler>& samplers() const final;

			const engine::vector<const RootConstant*>& root_constants() const final;

            const engine::vector<ShaderInputParameter>& inputParameters(int permutationIndex) const final;

            const engine::vector<Binding>& srvBindings() const final;
            const engine::vector<Binding>& uavBindings() const final;

        private:
            friend class {{shader_pipeline_configuration_class}};
            uint32_t descriptorCount() const;
            engine::vector<Shader::ConstantRange> m_constantRange;
            engine::vector<ShaderInputParameter> m_inputParameters[{{shader_pipeline_permutation_count}}];

            uint64_t hash(uint64_t hash = FnvOffsetBasis) const;

            int setStartIndex() const { return {{set_start_index}}; };
            int setCount() const { return {{set_count}}; };

		private:
			mutable engine::vector<TextureSRV> m_texture_srvs;
			mutable engine::vector<TextureUAV> m_texture_uavs;
            mutable engine::vector<unsigned char> m_texture_srvs_is_cube;
            mutable engine::vector<unsigned char> m_texture_uavs_is_cube;
            mutable engine::vector<Format> m_texture_srvs_format;
            mutable engine::vector<Format> m_texture_uavs_format;
			mutable engine::vector<BufferSRV> m_buffer_srvs;
			mutable engine::vector<BufferUAV> m_buffer_uavs;
            mutable engine::vector<RaytracingAccelerationStructure> m_acceleration_structures;
			mutable engine::vector<unsigned char> m_buffer_srvs_is_structured;
			mutable engine::vector<unsigned char> m_buffer_uavs_is_structured;
            mutable engine::vector<Format> m_buffer_srvs_format;
            mutable engine::vector<Format> m_buffer_uavs_format;
			mutable engine::vector<const BindlessTextureSRV*> m_bindless_texture_srvs;
            mutable engine::vector<const BindlessTextureUAV*> m_bindless_texture_uavs;
			mutable engine::vector<const BindlessBufferSRV*> m_bindless_buffer_srvs;
            mutable engine::vector<const BindlessBufferUAV*> m_bindless_buffer_uavs;
			mutable engine::vector<Sampler> m_samplers;
			mutable engine::vector<const RootConstant*> m_root_constants;

            mutable engine::vector<Binding> m_srvBindings;
            mutable engine::vector<Binding> m_uavBindings;
            mutable engine::vector<Binding> m_accelerationStructureBindings;

			mutable engine::vector<uint64_t> m_resourceIds;
			mutable SpookyHash m_hashEngine;
        };
    }
}