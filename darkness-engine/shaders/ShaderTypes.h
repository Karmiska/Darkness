#pragma once

#include "tools/ByteRange.h"
#include "engine/primitives/Matrix4.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/Format.h"
#include "shaders/ShaderPodTypes.h"
#include <cstdint>
#include "containers/memory.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    namespace implementation
    {
        class PipelineImplDX12;
        class PipelineImplVulkan;
        class CommandListImplDX12;
        class CommandListImplVulkan;
        class DescriptorHeapImplDX12;
        class DescriptorHeapImplVulkan;
        class PipelineShadersDX12;
        class PipelineShadersVulkan;
        class PipelineRootSignatureDX12;
        class PipelineRootSignatureVulkan;
    }
    class ShaderBinary;
    class ShaderStorage;
    class Device;
    class Sampler;
    class TextureSRV;
    class TextureUAV;
    class Buffer;
    class BufferSRV;
    class BufferUAV;
    class BufferCBV;
    class RaytracingAccelerationStructure;
    class CommandList;
	class BufferCBVOwner;

    class BindlessTextureSRV;
    class BindlessTextureUAV;
    class BindlessBufferSRV;
    class BindlessBufferUAV;
	class RootConstant;

    namespace shaders
    {
        constexpr uint64_t FnvPrime = 1099511628211u;
        constexpr uint64_t FnvOffsetBasis = 14695981039346656037u;
        uint64_t fnv1aHash(uint64_t value, uint64_t hash = FnvOffsetBasis);

		constexpr uint64_t ResourceHashSeed = 1009;
		constexpr uint64_t ResourceHashFactor = 9176;

        struct ShaderInputParameter
        {
            engine::string name;
            engine::string semanticName;
            engine::string type;
        };

        enum class BindingType
        {
            SRVTexture,
            SRVBuffer,
            UAVTexture,
            UAVBuffer,
            BindlessSRVTexture,
            BindlessSRVBuffer,
            BindlessUAVTexture,
            BindlessUAVBuffer,
			RaytracingAccelerationStructure
        };

        struct Binding
        {
            BindingType type;
            uint32_t index;
            engine::ResourceDimension dimension;
            Format format;
        };

        class Shader
        {
        public:
            virtual ~Shader() {};
        protected:
            friend class implementation::PipelineImplDX12;
            friend class implementation::PipelineImplVulkan;
            friend class implementation::PipelineShadersDX12;
            friend class implementation::PipelineShadersVulkan;
            friend class implementation::PipelineRootSignatureDX12;
            friend class implementation::PipelineRootSignatureVulkan;
            friend class implementation::CommandListImplDX12;
            friend class implementation::CommandListImplVulkan;
            friend class implementation::DescriptorHeapImplDX12;
            friend class implementation::DescriptorHeapImplVulkan;
            friend class engine::CommandList;
            
			virtual engine::ResourceDimension textureDimension(const engine::string& name) const = 0;
			virtual size_t samplerCount() const = 0;

            virtual int currentPermutationId() const = 0;
            virtual engine::shared_ptr<const ShaderBinary> load(const Device& device, ShaderStorage& storage, GraphicsApi api) const = 0;
            virtual const engine::vector<TextureSRV>& texture_srvs() const = 0;
            virtual const engine::vector<TextureUAV>& texture_uavs() const = 0;
            virtual engine::vector<TextureSRV>& texture_srvs() = 0;
            virtual engine::vector<TextureUAV>& texture_uavs() = 0;
            virtual const engine::vector<unsigned char>& texture_srvs_is_cube() const = 0;
            virtual const engine::vector<unsigned char>& texture_uavs_is_cube() const = 0;
            virtual const engine::vector<Format>& texture_srvs_format() const = 0;
            virtual const engine::vector<Format>& texture_uavs_format() const = 0;
            virtual const engine::vector<BufferSRV>& buffer_srvs() const = 0;
            virtual const engine::vector<BufferUAV>& buffer_uavs() const = 0;
            virtual engine::vector<BufferSRV>& buffer_srvs() = 0;
            virtual engine::vector<BufferUAV>& buffer_uavs() = 0;
            virtual const engine::vector<RaytracingAccelerationStructure>& acceleration_structures() const = 0;
			virtual const engine::vector<unsigned char>& buffer_srvs_is_structured() const = 0;
			virtual const engine::vector<unsigned char>& buffer_uavs_is_structured() const = 0;
            virtual const engine::vector<Format>& buffer_srvs_format() const = 0;
            virtual const engine::vector<Format>& buffer_uavs_format() const = 0;

            virtual const engine::vector<const BindlessTextureSRV*>& bindless_texture_srvs() const = 0;
            virtual const engine::vector<const BindlessTextureUAV*>& bindless_texture_uavs() const = 0;
            virtual const engine::vector<const BindlessBufferSRV*>& bindless_buffer_srvs() const = 0;
            virtual const engine::vector<const BindlessBufferUAV*>& bindless_buffer_uavs() const = 0;

            virtual engine::vector<const BindlessTextureSRV*>& bindless_texture_srvs() = 0;
            virtual engine::vector<const BindlessTextureUAV*>& bindless_texture_uavs() = 0;
            virtual engine::vector<const BindlessBufferSRV*>& bindless_buffer_srvs() = 0;
            virtual engine::vector<const BindlessBufferUAV*>& bindless_buffer_uavs() = 0;

			virtual const engine::vector<const RootConstant*>& root_constants() const = 0;

            virtual const engine::vector<Binding>& srvBindings() const = 0;
            virtual const engine::vector<Binding>& uavBindings() const = 0;

            virtual void setDebugBuffer(BufferUAV debugOutputBuffer) = 0;
            virtual bool hasDebugBuffer() const = 0;
            virtual bool debugBufferIsSet() const = 0;

            struct ConstantRange
            {
                tools::ByteRange range;
                engine::shared_ptr<BufferCBVOwner> buffer;
                const char* name;
            };
            virtual engine::vector<ConstantRange>& constants() = 0;
            virtual const engine::vector<Sampler>& samplers() const = 0;
            virtual const engine::vector<ShaderInputParameter>& inputParameters(int permutationIndex) const = 0;

			virtual engine::string shaderFilePath() const = 0;

            // returns the SET start index.
            virtual int setStartIndex() const = 0;
            virtual int setCount() const = 0;
        };

        class VertexShader : public Shader
        {};

        class PixelShader : public Shader
        {};

        class GeometryShader : public Shader
        {};

        class HullShader : public Shader
        {};

        class DomainShader : public Shader
        {};

        class ComputeShader : public Shader
        {};

		class RaygenerationShader : public Shader
		{};

		class IntersectionShader : public Shader
		{};

		class MissShader : public Shader
		{};

		class AnyHitShader : public Shader
		{};

		class ClosestHitShader : public Shader
		{};

        class AmplificationShader : public Shader
        {};

        class MeshShader : public Shader
        {};

        class PipelineConfiguration
        {
        public:
            virtual ~PipelineConfiguration() {};
            virtual bool hasVertexShader() const = 0;
            virtual bool hasPixelShader() const = 0;
            virtual bool hasGeometryShader() const = 0;
            virtual bool hasHullShader() const = 0;
            virtual bool hasDomainShader() const = 0;
            virtual bool hasComputeShader() const = 0;

			virtual bool hasRaygenerationShader() const = 0;
			virtual bool hasIntersectionShader() const = 0;
			virtual bool hasMissShader() const = 0;
			virtual bool hasAnyHitShader() const = 0;
			virtual bool hasClosestHitShader() const = 0;

            virtual bool hasAmplificationShader() const = 0;
            virtual bool hasMeshShader() const = 0;

            virtual uint32_t descriptorCount() const = 0;
            virtual uint64_t hash(uint64_t hash = FnvOffsetBasis) const = 0;

            virtual const Shader* vertexShader() const = 0;
            virtual const Shader* pixelShader() const = 0;
            virtual const Shader* geometryShader() const = 0;
            virtual const Shader* hullShader() const = 0;
            virtual const Shader* domainShader() const = 0;
            virtual const Shader* computeShader() const = 0;

			virtual const Shader* raygenerationShader() const = 0;
			virtual const Shader* intersectionShader() const = 0;
			virtual const Shader* missShader() const = 0;
			virtual const Shader* anyHitShader() const = 0;
			virtual const Shader* closestHitShader() const = 0;

            virtual const Shader* amplificationShader() const = 0;
            virtual const Shader* meshShader() const = 0;

            virtual const char* pipelineName() const = 0;
        };
    }

    Float4x4 fromMatrix(Matrix4f mat);
}
