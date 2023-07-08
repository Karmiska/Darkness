#include "{{pipeline_interface_filepath}}"
#include "tools/Debug.h"

namespace engine
{
    namespace shaders
    {
		{{pipeline_type_name}}::{{pipeline_type_name}}()
		{}

        bool {{pipeline_type_name}}::hasVertexShader() const
        {
            return {{chas_vertex_shader}};
        }

        bool {{pipeline_type_name}}::hasPixelShader() const
        {
            return {{chas_pixel_shader}};
        }

        bool {{pipeline_type_name}}::hasGeometryShader() const
        {
            return {{chas_geometry_shader}};
        }

        bool {{pipeline_type_name}}::hasHullShader() const
        {
            return {{chas_hull_shader}};
        }

        bool {{pipeline_type_name}}::hasDomainShader() const
        {
            return {{chas_domain_shader}};
        }

        bool {{pipeline_type_name}}::hasComputeShader() const
        {
            return {{chas_compute_shader}};
        }

		bool {{pipeline_type_name}}::hasRaygenerationShader() const
        {
            return {{chas_raygeneration_shader}};
        }

		bool {{pipeline_type_name}}::hasIntersectionShader() const
        {
            return {{chas_intersection_shader}};
        }

		bool {{pipeline_type_name}}::hasMissShader() const
        {
            return {{chas_miss_shader}};
        }

		bool {{pipeline_type_name}}::hasAnyHitShader() const
        {
            return {{chas_anyhit_shader}};
        }

		bool {{pipeline_type_name}}::hasClosestHitShader() const
        {
            return {{chas_closesthit_shader}};
        }

        bool {{pipeline_type_name}}::hasAmplificationShader() const
        {
            return {{chas_amplification_shader}};
        }

        bool {{pipeline_type_name}}::hasMeshShader() const
        {
            return {{chas_mesh_shader}};
        }

        uint32_t {{pipeline_type_name}}::descriptorCount() const
        {
            return 
                {% if has_vertex_shader -%}vs.descriptorCount()+{% endif %}
                {% if has_pixel_shader -%}ps.descriptorCount()+{% endif %}
                {% if has_geometry_shader -%}gs.descriptorCount()+{% endif %}
                {% if has_hull_shader -%}hs.descriptorCount()+{% endif %}
                {% if has_domain_shader -%}ds.descriptorCount()+{% endif %}
                {% if has_compute_shader -%}cs.descriptorCount()+{% endif %}
				{% if has_raygeneration_shader -%}rg.descriptorCount()+{% endif %}
				{% if has_intersection_shader -%}is.descriptorCount()+{% endif %}
				{% if has_miss_shader -%}ms.descriptorCount()+{% endif %}
				{% if has_anyhit_shader -%}ah.descriptorCount()+{% endif %}
				{% if has_closesthit_shader -%}ch.descriptorCount()+{% endif %}
                {% if has_amplification_shader -%}amp.descriptorCount()+{% endif %}
                {% if has_mesh_shader -%}mesh.descriptorCount()+{% endif %}
                0;
        }

        uint64_t {{pipeline_type_name}}::hash(uint64_t hash) const
        {
            {% if has_vertex_shader -%}hash = vs.hash(hash);{% endif %}
            {% if has_pixel_shader -%}hash = ps.hash(hash);{% endif %}
            {% if has_geometry_shader -%}hash = gs.hash(hash);{% endif %}
            {% if has_hull_shader -%}hash = hs.hash(hash);{% endif %}
            {% if has_domain_shader -%}hash = ds.hash(hash);{% endif %}
            {% if has_compute_shader -%}hash = cs.hash(hash);{% endif %}
			{% if has_raygeneration_shader -%}hash = rg.hash(hash);{% endif %}
			{% if has_intersection_shader -%}hash = is.hash(hash);{% endif %}
			{% if has_miss_shader -%}hash = ms.hash(hash);{% endif %}
			{% if has_anyhit_shader -%}hash = ah.hash(hash);{% endif %}
			{% if has_closesthit_shader -%}hash = ch.hash(hash);{% endif %}
            {% if has_amplification_shader -%}hash = amp.hash(hash);{% endif %}
            {% if has_mesh_shader -%}hash = mesh.hash(hash);{% endif %}
			return hash;
        }

        const Shader* {{pipeline_type_name}}::vertexShader() const
        {
            {% if has_vertex_shader -%}
            const Shader* shader = &vs;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::pixelShader() const
        {
            {% if has_pixel_shader -%}
            const Shader* shader = &ps;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::geometryShader() const
        {
            {% if has_geometry_shader -%}
            const Shader* shader = &gs;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::hullShader() const
        {
            {% if has_hull_shader -%}
            const Shader* shader = &hs;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::domainShader() const
        {
            {% if has_domain_shader -%}
            const Shader* shader = &ds;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::computeShader() const
        {
            {% if has_compute_shader -%}
            const Shader* shader = &cs;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

		const Shader* {{pipeline_type_name}}::raygenerationShader() const
        {
            {% if has_raygeneration_shader -%}
            const Shader* shader = &rg;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

		const Shader* {{pipeline_type_name}}::intersectionShader() const
        {
            {% if has_intersection_shader -%}
            const Shader* shader = &is;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

		const Shader* {{pipeline_type_name}}::missShader() const
        {
            {% if has_miss_shader -%}
            const Shader* shader = &ms;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

		const Shader* {{pipeline_type_name}}::anyHitShader() const
        {
            {% if has_anyhit_shader -%}
            const Shader* shader = &ah;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

		const Shader* {{pipeline_type_name}}::closestHitShader() const
        {
            {% if has_closesthit_shader -%}
            const Shader* shader = &ch;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::amplificationShader() const
        {
            {% if has_amplification_shader -%}
            const Shader* shader = &amp;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const Shader* {{pipeline_type_name}}::meshShader() const
        {
            {% if has_mesh_shader -%}
            const Shader* shader = &mesh;
            ASSERT(shader);
            return shader;
            {% else %}
            return nullptr;
            {% endif %}
        }

        const char* {{pipeline_type_name}}::pipelineName() const
        {
            return "{{pipeline_type_name}}";
        }
    }
}