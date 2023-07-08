#pragma once

#include "shaders/ShaderTypes.h"
#include "containers/memory.h"
{% if has_vertex_shader %}#include "{{vertex_shader_if}}"{% endif %}
{% if has_pixel_shader %}#include "{{pixel_shader_if}}"{% endif %}
{% if has_geometry_shader %}#include "{{geometry_shader_if}}"{% endif %}
{% if has_hull_shader %}#include "{{hull_shader_if}}"{% endif %}
{% if has_domain_shader %}#include "{{domain_shader_if}}"{% endif %}
{% if has_compute_shader %}#include "{{compute_shader_if}}"{% endif %}

{% if has_raygeneration_shader %}#include "{{raygeneration_shader_if}}"{% endif %}
{% if has_intersection_shader %}#include "{{intersection_shader_if}}"{% endif %}
{% if has_miss_shader %}#include "{{miss_shader_if}}"{% endif %}
{% if has_anyhit_shader %}#include "{{anyhit_shader_if}}"{% endif %}
{% if has_closesthit_shader %}#include "{{closesthit_shader_if}}"{% endif %}

{% if has_amplification_shader %}#include "{{amplification_shader_if}}"{% endif %}
{% if has_mesh_shader %}#include "{{mesh_shader_if}}"{% endif %}

namespace engine
{
    namespace implementation
    {
        class PipelineImpl;
    }

    namespace shaders
    {
        class Shader;

        class {{pipeline_type_name}} : public PipelineConfiguration
        {
        public:
			{{pipeline_type_name}}();
			{{pipeline_type_name}}(const {{pipeline_type_name}}&) = default;
			{{pipeline_type_name}}({{pipeline_type_name}}&&) = default;
			{{pipeline_type_name}}& operator=(const {{pipeline_type_name}}&) = default;
			{{pipeline_type_name}}& operator=({{pipeline_type_name}}&&) = default;

            {% if has_vertex_shader -%}{{vertex_shader_type}} vs;{% endif %}
            {% if has_pixel_shader -%}{{pixel_shader_type}} ps;{% endif %}
            {% if has_geometry_shader -%}{{geometry_shader_type}} gs;{% endif %}
            {% if has_hull_shader -%}{{hull_shader_type}} hs;{% endif %}
            {% if has_domain_shader -%}{{domain_shader_type}} ds;{% endif %}
            {% if has_compute_shader -%}{{compute_shader_type}} cs;{% endif %}

			{% if has_raygeneration_shader -%}{{raygeneration_shader_type}} rg;{% endif %}
			{% if has_intersection_shader -%}{{intersection_shader_type}} is;{% endif %}
			{% if has_miss_shader -%}{{miss_shader_type}} ms;{% endif %}
			{% if has_anyhit_shader -%}{{anyhit_shader_type}} ah;{% endif %}
			{% if has_closesthit_shader -%}{{closesthit_shader_type}} ch;{% endif %}

            {% if has_amplification_shader -%}{{amplification_shader_type}} amp;{% endif %}
            {% if has_mesh_shader -%}{{mesh_shader_type}} mesh;{% endif %}

            uint32_t descriptorCount() const override;
            uint64_t hash(uint64_t hash = ResourceHashSeed) const override;

        private:
            friend class PipelineImpl;
            const Shader* vertexShader() const override;
            const Shader* pixelShader() const override;
            const Shader* geometryShader() const override;
            const Shader* hullShader() const override;
            const Shader* domainShader() const override;
            const Shader* computeShader() const override;

			const Shader* raygenerationShader() const override;
			const Shader* intersectionShader() const override;
			const Shader* missShader() const override;
			const Shader* anyHitShader() const override;
			const Shader* closestHitShader() const override;

            const Shader* amplificationShader() const override;
            const Shader* meshShader() const override;

            bool hasVertexShader() const override;
            bool hasPixelShader() const override;
            bool hasGeometryShader() const override;
            bool hasHullShader() const override;
            bool hasDomainShader() const override;
            bool hasComputeShader() const override;

			bool hasRaygenerationShader() const override;
			bool hasIntersectionShader() const override;
			bool hasMissShader() const override;
			bool hasAnyHitShader() const override;
			bool hasClosestHitShader() const override;

            bool hasAmplificationShader() const override;
            bool hasMeshShader() const override;

            const char* pipelineName() const override;
        };
    }
}