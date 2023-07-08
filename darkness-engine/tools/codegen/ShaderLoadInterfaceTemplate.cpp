#include "{{ShaderLoadInterfaceHeader}}"
#include "engine/graphics/ShaderStorage.h"
#include "engine/graphics/Sampler.h"
#include "tools/ByteRange.h"
#include "tools/Debug.h"
#include "containers/memory.h"
#include "engine/graphics/ShaderLocator.h"

namespace engine
{
    namespace shaders
    {
#pragma warning( push )
#pragma warning( disable : 4702 )
        int {{ShaderClass}}::currentPermutationId() const
        {
            {% for permutation in permutations %}
            if((true)
            {% for permutationItem in permutation.list %}
            && ({{permutationItem.variableName}} == {{permutationItem.value}})
            {% endfor %}
            )
            {
                return {{loop.index}};
            }
            {% endfor %}
            {% if length(permutations) == 0 %}
            return 0;
            {% endif %}
            ASSERT(false, "Could not load the permutation necessary. This is a bug.");
            return 0;
        }

        engine::shared_ptr<const ShaderBinary> {{ShaderClass}}::load(const Device& device, ShaderStorage& storage, GraphicsApi api) const
        {
            auto corePath = ShaderLocator::instance().getCoreShaderPath(api);
            {% for permutation in permutations %}
            if((true)
            {% for permutationItem in permutation.list %}
            && ({{permutationItem.variableName}} == {{permutationItem.value}})
            {% endfor %}
            )
            {
                return storage.loadShader(device, 
                    pathJoin(
                        corePath,
                        "{{BasePathAndFile}}_{{permutation.id}}{{BaseExt}}"), 
                    pathJoin(
                        corePath,
                        "{{BasePathAndFile}}_{{permutation.id}}.support"), {{loop.index}}-1, {
                    {% for define in permutation.defines %}
                    "{{define}}"{% if not loop.is_last%},{% endif %}
                    {% endfor %}
                    }, api);
            }
            {% endfor %}
            {% if length(permutations) == 0 %}
            return storage.loadShader(device, 
                pathJoin(
                    corePath,
                    "{{ShaderBinaryPath}}"), 
                pathJoin(
                    corePath,
                    "{{BasePathAndFile}}.support"), -1, {}, api);
            {% endif %}
            ASSERT(false, "Could not load the permutation necessary. This is a bug.");
            return {};
        }
#pragma warning( pop )


        {{ShaderClass}}::{{ShaderClass}}()
            : m_constantRange{
            {% if has_constants %}
            {% for item in constant_structures %}
                ConstantRange{
                    tools::ByteRange(
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)),
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)) + sizeof({{item.name}})),
                    nullptr,
                    "{{item.name}}"
                }
                {% if not loop.is_last %}
                ,
                {% endif %}
            {% endfor %}
            {% endif %}
            }
            , m_inputParameters{
            {% for item in input_parameters%}
                {
                {% for parameter in item %}
                ShaderInputParameter{"{{parameter.name}}", "{{parameter.semantic}}", "{{parameter.type}}"}
                {% if not loop.is_last %}
                ,
                {% endif %}
                {% endfor %}
                }
                {% if not loop.is_last %}
                ,
                {% endif %}
            {% endfor %}
            }

			{% if has_root_constants %}
			,
			{% for item in root_constants%}
			{{item.identifier}}("{{item.identifier}}")
			{% if not loop.is_last %}
                ,
                {% endif %}
			{% endfor%}
			{% endif %}

            , m_texture_srvs({{length(texture_srvs)}})
			, m_texture_uavs({{length(texture_uavs)}})
            , m_texture_srvs_is_cube({{length(texture_srvs)}})
			, m_texture_uavs_is_cube({{length(texture_uavs)}})
            , m_texture_srvs_format({{length(texture_srvs)}})
			, m_texture_uavs_format({{length(texture_uavs)}})
			, m_buffer_srvs({{length(buffer_srvs)}})
			, m_buffer_uavs({{length(buffer_uavs)}})
            , m_acceleration_structures({{length(acceleration_structures)}})
			, m_buffer_srvs_is_structured({{length(buffer_srvs)}})
			, m_buffer_uavs_is_structured({{length(buffer_uavs)}})
            , m_buffer_srvs_format({{length(buffer_srvs)}})
			, m_buffer_uavs_format({{length(buffer_uavs)}})
			, m_bindless_texture_srvs({{length(bindless_texture_srvs)}})
			, m_bindless_texture_uavs({{length(bindless_texture_uavs)}})
			, m_bindless_buffer_srvs({{length(bindless_buffer_srvs)}})
			, m_bindless_buffer_uavs({{length(bindless_buffer_uavs)}})
			, m_samplers({{length(samplers)}})
			, m_root_constants({{length(root_constants)}})
			, m_resourceIds(
				{% if has_texture_srvs %}{{length(texture_srvs)}} +{% endif %}
				{% if has_texture_uavs %}{{length(texture_uavs)}} +{% endif %}
				{% if has_buffer_srvs %}{{length(buffer_srvs)}} +{% endif %}
				{% if has_buffer_uavs %}{{length(buffer_uavs)}} +{% endif %}
                {% if has_acceleration_structures %}{{length(acceleration_structures)}} +{% endif %}
				{% if has_bindless_texture_srvs %}{{length(bindless_texture_srvs)}} +{% endif %}
				{% if has_bindless_texture_uavs %}{{length(bindless_texture_uavs)}} +{% endif %}
				{% if has_bindless_buffer_srvs %}{{length(bindless_buffer_srvs)}} +{% endif %}
				{% if has_bindless_buffer_uavs %}{{length(bindless_buffer_uavs)}} +{% endif %}
				1)
			, m_hashEngine{}
        {
			m_hashEngine.Init(123, 456);

            {% for item in srvs_bindings%}
            m_srvBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in uavs_bindings%}
            m_uavBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in acceleration_bindings %}
            m_accelerationStructureBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}
		}

		engine::string {{ShaderClass}}::shaderFilePath() const
		{
			auto corePath = ShaderLocator::instance().getCoreShaderPath();
			return pathJoin(
                corePath, "{{BasePathAndFile}}");
		}

#pragma warning( push )
#pragma warning( disable : 4100 )
        {{ShaderClass}}::{{ShaderClass}}(const {{ShaderClass}}& cl)
            : m_constantRange{
            {% if has_constants %}
            {% for item in constant_structures %}
                ConstantRange{
                    tools::ByteRange(
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)),
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)) + sizeof({{item.name}})),
                    nullptr,
                    "{{item.name}}"
                }
                {% if not loop.is_last %}
                ,
                {% endif %}
            {% endfor %}
            {% endif %}
            }
			{% if has_root_constants %}
			,
			{% for item in root_constants%}
			{{item.identifier}}("{{item.identifier}}")
			{% if not loop.is_last %}
                ,
                {% endif %}
			{% endfor%}
			{% endif %}

			, m_texture_srvs({{length(texture_srvs)}})
			, m_texture_uavs({{length(texture_uavs)}})
            , m_texture_srvs_is_cube({{length(texture_srvs)}})
			, m_texture_uavs_is_cube({{length(texture_uavs)}})
            , m_texture_srvs_format({{length(texture_srvs)}})
			, m_texture_uavs_format({{length(texture_uavs)}})
			, m_buffer_srvs({{length(buffer_srvs)}})
			, m_buffer_uavs({{length(buffer_uavs)}})
            , m_acceleration_structures({{length(acceleration_structures)}})
			, m_buffer_srvs_is_structured({{length(buffer_srvs)}})
			, m_buffer_uavs_is_structured({{length(buffer_uavs)}})
            , m_buffer_srvs_format({{length(buffer_srvs)}})
			, m_buffer_uavs_format({{length(buffer_uavs)}})
			, m_bindless_texture_srvs({{length(bindless_texture_srvs)}})
			, m_bindless_texture_uavs({{length(bindless_texture_uavs)}})
			, m_bindless_buffer_srvs({{length(bindless_buffer_srvs)}})
			, m_bindless_buffer_uavs({{length(bindless_buffer_uavs)}})
			, m_samplers({{length(samplers)}})
			, m_root_constants({{length(root_constants)}})

			, m_resourceIds(
				{% if has_texture_srvs %}{{length(texture_srvs)}} +{% endif %}
				{% if has_texture_uavs %}{{length(texture_uavs)}} +{% endif %}
				{% if has_buffer_srvs %}{{length(buffer_srvs)}} +{% endif %}
				{% if has_buffer_uavs %}{{length(buffer_uavs)}} +{% endif %}
                {% if has_acceleration_structures %}{{length(acceleration_structures)}} +{% endif %}
				{% if has_bindless_texture_srvs %}{{length(bindless_texture_srvs)}} +{% endif %}
				{% if has_bindless_texture_uavs %}{{length(bindless_texture_uavs)}} +{% endif %}
				{% if has_bindless_buffer_srvs %}{{length(bindless_buffer_srvs)}} +{% endif %}
				{% if has_bindless_buffer_uavs %}{{length(bindless_buffer_uavs)}} +{% endif %}
				1)
			, m_hashEngine{}
        {
            for (size_t i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = cl.m_constantRange[i].buffer;
            }

            {% for item in texture_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in texture_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in buffer_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in buffer_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in acceleration_structures %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_texture_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_texture_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_buffer_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_buffer_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in samplers %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in options %}
            {{item.value}} = cl.{{item.value}};
            {% endfor -%}

            {% for item in enums -%}
            {{item.name_lower}} = cl.{{item.name_lower}};
            {% endfor -%}
            

            m_hashEngine.Init(123, 456);

            {% for item in srvs_bindings%}
            m_srvBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in uavs_bindings%}
            m_uavBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in acceleration_bindings %}
            m_accelerationStructureBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}
        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        {{ShaderClass}}::{{ShaderClass}}({{ShaderClass}}&& cl)
            : m_constantRange{
            {% if has_constants %}
            {% for item in constant_structures %}
                ConstantRange{
                    tools::ByteRange(
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)),
                        reinterpret_cast<const uint8_t*>(static_cast<const {{item.name}}*>(this)) + sizeof({{item.name}})),
                    nullptr,
                    "{{item.name}}"
                }
                {% if not loop.is_last %}
                ,
                {% endif %}
            {% endfor %}
            {% endif %}
            }
			{% if has_root_constants %}
			,
			{% for item in root_constants%}
			{{item.identifier}}("{{item.identifier}}")
			{% if not loop.is_last %}
                ,
                {% endif %}
			{% endfor%}
			{% endif %}

			, m_texture_srvs({{length(texture_srvs)}})
			, m_texture_uavs({{length(texture_uavs)}})
            , m_texture_srvs_is_cube({{length(texture_srvs)}})
			, m_texture_uavs_is_cube({{length(texture_uavs)}})
            , m_texture_srvs_format({{length(texture_srvs)}})
			, m_texture_uavs_format({{length(texture_uavs)}})
			, m_buffer_srvs({{length(buffer_srvs)}})
			, m_buffer_uavs({{length(buffer_uavs)}})
            , m_acceleration_structures({{length(acceleration_structures)}})
			, m_buffer_srvs_is_structured({{length(buffer_srvs)}})
			, m_buffer_uavs_is_structured({{length(buffer_uavs)}})
            , m_buffer_srvs_format({{length(buffer_srvs)}})
			, m_buffer_uavs_format({{length(buffer_uavs)}})
			, m_bindless_texture_srvs({{length(bindless_texture_srvs)}})
			, m_bindless_texture_uavs({{length(bindless_texture_uavs)}})
			, m_bindless_buffer_srvs({{length(bindless_buffer_srvs)}})
			, m_bindless_buffer_uavs({{length(bindless_buffer_uavs)}})
			, m_samplers({{length(samplers)}})
			, m_root_constants({{length(root_constants)}})

			, m_resourceIds(
				{% if has_texture_srvs %}{{length(texture_srvs)}} +{% endif %}
				{% if has_texture_uavs %}{{length(texture_uavs)}} +{% endif %}
				{% if has_buffer_srvs %}{{length(buffer_srvs)}} +{% endif %}
				{% if has_buffer_uavs %}{{length(buffer_uavs)}} +{% endif %}
                {% if has_acceleration_structures %}{{length(acceleration_structures)}} +{% endif %}
				{% if has_bindless_texture_srvs %}{{length(bindless_texture_srvs)}} +{% endif %}
				{% if has_bindless_texture_uavs %}{{length(bindless_texture_uavs)}} +{% endif %}
				{% if has_bindless_buffer_srvs %}{{length(bindless_buffer_srvs)}} +{% endif %}
				{% if has_bindless_buffer_uavs %}{{length(bindless_buffer_uavs)}} +{% endif %}
				1)
			, m_hashEngine{}
        {
            for (size_t i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = std::move(cl.m_constantRange[i].buffer);
            }

            {% for item in texture_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in texture_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in buffer_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in buffer_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in acceleration_structures %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_texture_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_texture_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_buffer_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_buffer_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in samplers %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in options %}
            {{item.value}} = std::move(cl.{{item.value}});
            {% endfor -%}

            {% for item in enums -%}
            {{item.name_lower}} = cl.{{item.name_lower}};
            {% endfor -%}


            m_hashEngine.Init(123, 456);

            {% for item in srvs_bindings%}
            m_srvBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in uavs_bindings%}
            m_uavBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}

            {% for item in acceleration_bindings %}
            m_accelerationStructureBindings.emplace_back(Binding{ BindingType::{{item.type}}, {{item.index}}, engine::ResourceDimension::{{item.dimension}}, {{item.format}} });
			{% endfor%}
        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        {{ShaderClass}}& {{ShaderClass}}::operator=(const {{ShaderClass}}& cl)
        {
            for (size_t i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = cl.m_constantRange[i].buffer;
            }

            {% for item in texture_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in texture_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in buffer_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in buffer_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in acceleration_structures %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_texture_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_texture_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_buffer_srvs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in bindless_buffer_uavs %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in samplers %}
            {{item.identifier}} = cl.{{item.identifier}};
            {% endfor %}

            {% for item in options %}
            {{item.value}} = cl.{{item.value}};
            {% endfor -%}

            {% for item in enums -%}
            {{item.name_lower}} = cl.{{item.name_lower}};
            {% endfor -%}

			{% if has_root_constants %}
			{% for item in root_constants%}
			{{item.identifier}} = cl.{{item.identifier}};
			{% endfor%}
			{% endif %}

            return *this;
        }
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
        {{ShaderClass}}& {{ShaderClass}}::operator=({{ShaderClass}}&& cl)
        {
            for (size_t i = 0; i < m_constantRange.size(); ++i)
            {
                m_constantRange[i].buffer = std::move(cl.m_constantRange[i].buffer);
            }

            {% for item in texture_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in texture_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in buffer_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in buffer_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in acceleration_structures %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_texture_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_texture_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_buffer_srvs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in bindless_buffer_uavs %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in samplers %}
            {{item.identifier}} = std::move(cl.{{item.identifier}});
            {% endfor %}

            {% for item in options %}
            {{item.value}} = std::move(cl.{{item.value}});
            {% endfor -%}

            {% for item in enums -%}
            {{item.name_lower}} = cl.{{item.name_lower}};
            {% endfor -%}

			{% if has_root_constants %}
			{% for item in root_constants%}
			{{item.identifier}} = cl.{{item.identifier}};
			{% endfor%}
			{% endif %}

            return *this;
        }
#pragma warning( pop )

        engine::ResourceDimension {{ShaderClass}}::textureDimension(const engine::string&{% if length(dimensions) > 0 %} name{% endif %}) const
        {
            {% for item in dimensions %}
            if("{{item.identifier}}" == name) return engine::ResourceDimension::{{item.dimension}};
            {% endfor %}
            return engine::ResourceDimension::Unknown;
        }

		size_t {{ShaderClass}}::samplerCount() const
		{
			return {{length(samplers)}};
		}

        const engine::vector<TextureSRV>& {{ShaderClass}}::texture_srvs() const
        {
			{% if has_texture_srvs %}
			TextureSRV* ptr = &m_texture_srvs[0];
			{% endif %}
            {% for item in texture_srvs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
            {% endfor %}
            return m_texture_srvs;
        }

        const engine::vector<TextureUAV>& {{ShaderClass}}::texture_uavs() const
        {
			{% if has_texture_uavs %}
			TextureUAV* ptr = &m_texture_uavs[0];
			{% endif %}
            {% for item in texture_uavs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_uavs;
        }

        engine::vector<TextureSRV>& {{ShaderClass}}::texture_srvs()
        {
			{% if has_texture_srvs %}
			TextureSRV* ptr = &m_texture_srvs[0];
			{% endif %}
            {% for item in texture_srvs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
            {% endfor %}
            return m_texture_srvs;
        }

        engine::vector<TextureUAV>& {{ShaderClass}}::texture_uavs()
        {
			{% if has_texture_uavs %}
			TextureUAV* ptr = &m_texture_uavs[0];
			{% endif %}
            {% for item in texture_uavs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_uavs;
        }

        const engine::vector<unsigned char>& {{ShaderClass}}::texture_srvs_is_cube() const
        {
			{% if has_texture_srvs %}
			unsigned char* ptr = &m_texture_srvs_is_cube[0];
			{% endif %}
            {% for item in texture_srvs %}
			*ptr = {{item.cube}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_srvs_is_cube;
        }

        const engine::vector<unsigned char>& {{ShaderClass}}::texture_uavs_is_cube() const
        {
			{% if has_texture_uavs %}
			unsigned char* ptr = &m_texture_uavs_is_cube[0];
			{% endif %}
            {% for item in texture_uavs %}
			*ptr = {{item.cube}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_uavs_is_cube;
        }

        const engine::vector<Format>& {{ShaderClass}}::texture_srvs_format() const
        {
			{% if has_texture_srvs %}
			Format* ptr = &m_texture_srvs_format[0];
			{% endif %}
            {% for item in texture_srvs %}
			*ptr = {{item.format}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_srvs_format;
        }

        const engine::vector<Format>& {{ShaderClass}}::texture_uavs_format() const
        {
			{% if has_texture_uavs %}
			Format* ptr = &m_texture_uavs_format[0];
			{% endif %}
            {% for item in texture_uavs %}
			*ptr = {{item.format}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_texture_uavs_format;
        }

        const engine::vector<BufferSRV>& {{ShaderClass}}::buffer_srvs() const
        {
			{% if has_buffer_srvs %}
			BufferSRV* ptr = &m_buffer_srvs[0];
			{% endif %}
            {% for item in buffer_srvs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_srvs;
        }

        const engine::vector<BufferUAV>& {{ShaderClass}}::buffer_uavs() const
        {
			{% if has_buffer_uavs %}
			BufferUAV* ptr = &m_buffer_uavs[0];
			{% endif %}
            {% for item in buffer_uavs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_uavs;
        }

        engine::vector<BufferSRV>& {{ShaderClass}}::buffer_srvs()
        {
			{% if has_buffer_srvs %}
			BufferSRV* ptr = &m_buffer_srvs[0];
			{% endif %}
            {% for item in buffer_srvs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_srvs;
        }

        engine::vector<BufferUAV>& {{ShaderClass}}::buffer_uavs()
        {
			{% if has_buffer_uavs %}
			BufferUAV* ptr = &m_buffer_uavs[0];
			{% endif %}
            {% for item in buffer_uavs %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_uavs;
        }

        const engine::vector<RaytracingAccelerationStructure>& {{ShaderClass}}::acceleration_structures() const
        {
			{% if has_acceleration_structures %}
            RaytracingAccelerationStructure* ptr = &m_acceleration_structures[0];
			{% endif %}
            {% for item in acceleration_structures %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_acceleration_structures;
        }

		const engine::vector<unsigned char>& {{ShaderClass}}::buffer_srvs_is_structured() const
        {
			{% if has_buffer_srvs %}
			unsigned char* ptr = &m_buffer_srvs_is_structured[0];
			{% endif %}
            {% for item in buffer_srvs %}
			*ptr = {{item.structured}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_srvs_is_structured;
        }

        const engine::vector<unsigned char>& {{ShaderClass}}::buffer_uavs_is_structured() const
        {
			{% if has_buffer_uavs %}
			unsigned char* ptr = &m_buffer_uavs_is_structured[0];
			{% endif %}
            {% for item in buffer_uavs %}
			*ptr = {{item.structured}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_uavs_is_structured;
        }

        const engine::vector<Format>& {{ShaderClass}}::buffer_srvs_format() const
        {
			{% if has_buffer_srvs %}
			Format* ptr = &m_buffer_srvs_format[0];
			{% endif %}
            {% for item in buffer_srvs %}
			*ptr = {{item.format}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_srvs_format;
        }

        const engine::vector<Format>& {{ShaderClass}}::buffer_uavs_format() const
        {
			{% if has_buffer_uavs %}
			Format* ptr = &m_buffer_uavs_format[0];
			{% endif %}
            {% for item in buffer_uavs %}
			*ptr = {{item.format}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_buffer_uavs_format;
        }

        const engine::vector<const BindlessTextureSRV*>& {{ShaderClass}}::bindless_texture_srvs() const
        {
			{% if has_bindless_texture_srvs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_texture_srvs %}
			m_bindless_texture_srvs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_texture_srvs;
        }

        const engine::vector<const BindlessTextureUAV*>& {{ShaderClass}}::bindless_texture_uavs() const
        {
			{% if has_bindless_texture_uavs %}
			uint32_t index = 0;
			const BindlessTextureUAV** ptr = &m_bindless_texture_uavs[0];
			{% endif %}
            {% for item in bindless_texture_uavs %}
			m_bindless_texture_uavs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_texture_uavs;
        }

        const engine::vector<const BindlessBufferSRV*>& {{ShaderClass}}::bindless_buffer_srvs() const
        {
			{% if has_bindless_buffer_srvs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_buffer_srvs %}
			m_bindless_buffer_srvs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_buffer_srvs;
        }

        const engine::vector<const BindlessBufferUAV*>& {{ShaderClass}}::bindless_buffer_uavs() const
        {
			{% if has_bindless_buffer_uavs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_buffer_uavs %}
			*m_bindless_buffer_uavs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_buffer_uavs;
        }

        engine::vector<const BindlessTextureSRV*>& {{ShaderClass}}::bindless_texture_srvs()
        {
			{% if has_bindless_texture_srvs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_texture_srvs %}
			m_bindless_texture_srvs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_texture_srvs;
        }

        engine::vector<const BindlessTextureUAV*>& {{ShaderClass}}::bindless_texture_uavs()
        {
			{% if has_bindless_texture_uavs %}
			uint32_t index = 0;
			const BindlessTextureUAV** ptr = &m_bindless_texture_uavs[0];
			{% endif %}
            {% for item in bindless_texture_uavs %}
			m_bindless_texture_uavs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_texture_uavs;
        }

        engine::vector<const BindlessBufferSRV*>& {{ShaderClass}}::bindless_buffer_srvs()
        {
			{% if has_bindless_buffer_srvs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_buffer_srvs %}
			m_bindless_buffer_srvs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_buffer_srvs;
        }

        engine::vector<const BindlessBufferUAV*>& {{ShaderClass}}::bindless_buffer_uavs()
        {
			{% if has_bindless_buffer_uavs %}
			uint32_t index = 0;
			{% endif %}
            {% for item in bindless_buffer_uavs %}
			*m_bindless_buffer_uavs[index] = &{{item.identifier}};
			{% if not loop.is_last %}
			++index;
            {% endif %}
			{% endfor %}
            return m_bindless_buffer_uavs;
        }

        engine::vector<Shader::ConstantRange>& {{ShaderClass}}::constants()
        {
            return m_constantRange;
        }

        const engine::vector<Sampler>& {{ShaderClass}}::samplers() const
        {
			{% if has_samplers %}
			Sampler* ptr = &m_samplers[0];
			{% endif %}
            {% for item in samplers %}
			*ptr = {{item.identifier}};
			{% if not loop.is_last %}
			++ptr;
            {% endif %}
			{% endfor %}
            return m_samplers;
        }

		const engine::vector<const RootConstant*>& {{ShaderClass}}::root_constants() const
		{ 
            {% for item in root_constants%}
            m_root_constants[{{loop.index}}] = &{{item.identifier}};
			{% endfor %}
            return m_root_constants;
		}

        const engine::vector<ShaderInputParameter>& {{ShaderClass}}::inputParameters(int permutationIndex) const
        {
            return m_inputParameters[permutationIndex];
        }

        const engine::vector<Binding>& {{ShaderClass}}::srvBindings() const
        {
            return m_srvBindings;
        }

        const engine::vector<Binding>& {{ShaderClass}}::uavBindings() const
        {
            return m_uavBindings;
        }

        uint64_t {{ShaderClass}}::hash(uint64_t hash) const
        {
            {% if has_texture_srvs %}
            {% for item in texture_srvs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_texture_uavs %}
            {% for item in texture_uavs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_buffer_srvs %}
            {% for item in buffer_srvs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_acceleration_structures %}
            {% for item in acceleration_structures %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_buffer_uavs %}
            {% for item in buffer_uavs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_bindless_texture_srvs %}
            {% for item in bindless_texture_srvs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_bindless_texture_uavs %}
            {% for item in bindless_texture_uavs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_bindless_buffer_srvs %}
            {% for item in bindless_buffer_srvs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            {% if has_bindless_buffer_uavs %}
            {% for item in bindless_buffer_uavs %}
			hash = fnv1aHash({{item.identifier}}.resourceId(), hash);
            {% endfor %}
            {% endif %}

            return hash;
        }

        uint32_t {{ShaderClass}}::descriptorCount() const
        {
            return {{descriptor_count}};
        }

        void {{ShaderClass}}::setDebugBuffer(BufferUAV
            {% if has_debug_output %}
            debugOutputBuffer
            {% endif %}
        )
        {
            {% if has_debug_output %}
            debugOutput = debugOutputBuffer;
            {% endif %}
        }

        bool {{ShaderClass}}::hasDebugBuffer() const
        {
            {% if has_debug_output %}
            return true;
            {% endif %}

            {% if not has_debug_output %}
            return false;
            {% endif %}
        }

        bool {{ShaderClass}}::debugBufferIsSet() const
        {
            {% if has_debug_output %}
            return debugOutput.valid();
            {% endif %}

            {% if not has_debug_output %}
            return false;
            {% endif %}
        }
    }
}