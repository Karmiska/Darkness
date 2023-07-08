#include "engine/graphics/dx12/DX12Pipeline.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Resources.h"
#include "engine/graphics/dx12/DX12ShaderBinary.h"
#include "engine/graphics/dx12/DX12Conversions.h"
#include "engine/graphics/dx12/DX12RootSignature.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12RootParameter.h"
#include "engine/graphics/dx12/DX12DescriptorHeap.h"
#include "engine/graphics/dx12/DX12Sampler.h"
#include "engine/graphics/dx12/DX12CommandList.h"

#include "engine/graphics/ShaderBinary.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Sampler.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/Barrier.h"

#include "tools/Debug.h"
#include "tools/hash/Hash.h"
#include "tools/PathTools.h"
#include "containers/memory.h"
#include <algorithm>

#ifndef _DURANGO
#include <D3d12shader.h>
#include <D3Dcompiler.h>
#endif

#include <sstream>
#include <inttypes.h>

using namespace tools;

#ifndef _DURANGO
#define DXIL_SUPPORT_ENABLED
#else
#define DXIL_SUPPORT_ENABLED
#endif

#undef PRINT_SIGNATURES_DESCRIPTORS
#ifdef PRINT_SIGNATURES_DESCRIPTORS
#include <fstream>
#include "tools/PathTools.h"
#include <inttypes.h>
#endif

//#include "DXRHelper.h"
#ifndef _DURANGO
#include "nv_helpers_dx12/RootSignatureGenerator.h"
#include "nv_helpers_dx12/RaytracingPipelineGenerator.h"
#include "nv_helpers_dx12/ShaderBindingTableGenerator.h"

#ifdef DXR_BUILD
#include "DXRHelper.h"
#endif
#endif

namespace engine
{
    namespace implementation
    {
#if 1
#ifdef PRINT_SIGNATURES_DESCRIPTORS
        engine::string rsPathFromShaderPath(const engine::string& filePath)
        {
            return filePath + ".rs";
        }

        void printRootSignatureFile(const engine::string& filePath)
        {
            LOG("RootSignature file for: %s", filePath.c_str());
            std::ifstream file(filePath, std::ios::in);
            if (file.is_open())
            {
                engine::string line;
                while (std::getline(file, line))
                {
                    LOG("%s", line.c_str());
                }
                file.close();
            }
        }
#endif

        PipelineShadersDX12::PipelineShadersDX12(
            Device & device,
            ShaderStorage & storage,
            std::function<void()> onChange,
            shaders::PipelineConfiguration * configuration)
            : m_device{ device }
            , m_storage{ storage }
            , m_onChange{ onChange }
            , m_cs{ nullptr }
            , m_vs{ nullptr }
            , m_ps{ nullptr }
            , m_hs{ nullptr }
            , m_gs{ nullptr }
            , m_ds{ nullptr }
            , m_rg{ nullptr }
            , m_is{ nullptr }
            , m_ms{ nullptr }
            , m_ah{ nullptr }
            , m_ch{ nullptr }
            , m_amp{ nullptr }
            , m_mesh{ nullptr }
        {
            loadShaders(configuration);
        }

        PipelineShadersDX12::~PipelineShadersDX12()
        {
            if (m_cs) m_cs->unregisterForChange(this);
            if (m_vs) m_vs->unregisterForChange(this);
            if (m_ps) m_ps->unregisterForChange(this);
            if (m_hs) m_hs->unregisterForChange(this);
            if (m_gs) m_gs->unregisterForChange(this);
            if (m_ds) m_ds->unregisterForChange(this);

            if (m_rg) m_rg->unregisterForChange(this);
            if (m_is) m_is->unregisterForChange(this);
            if (m_ms) m_ms->unregisterForChange(this);
            if (m_ah) m_ah->unregisterForChange(this);
            if (m_ch) m_ch->unregisterForChange(this);

            if (m_amp) m_amp->unregisterForChange(this);
            if (m_mesh) m_mesh->unregisterForChange(this);
        }

        const ShaderBinary* PipelineShadersDX12::cs() const { return m_cs.get(); }
        const ShaderBinary* PipelineShadersDX12::vs() const { return m_vs.get(); }
        const ShaderBinary* PipelineShadersDX12::ps() const { return m_ps.get(); }
        const ShaderBinary* PipelineShadersDX12::hs() const { return m_hs.get(); }
        const ShaderBinary* PipelineShadersDX12::gs() const { return m_gs.get(); }
        const ShaderBinary* PipelineShadersDX12::ds() const { return m_ds.get(); }

        const ShaderBinary* PipelineShadersDX12::rg() const { return m_rg.get(); }
        const ShaderBinary* PipelineShadersDX12::is() const { return m_is.get(); }
        const ShaderBinary* PipelineShadersDX12::ms() const { return m_ms.get(); }
        const ShaderBinary* PipelineShadersDX12::ah() const { return m_ah.get(); }
        const ShaderBinary* PipelineShadersDX12::ch() const { return m_ch.get(); }

        const ShaderBinary* PipelineShadersDX12::amp() const { return m_amp.get(); }
        const ShaderBinary* PipelineShadersDX12::mesh() const { return m_mesh.get(); }

        void PipelineShadersDX12::loadShaders(shaders::PipelineConfiguration* configuration)
        {
            if (configuration->hasComputeShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->computeShader()->shaderFilePath()));
#endif
                m_cs = configuration->computeShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_cs->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasVertexShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->vertexShader()->shaderFilePath()));
#endif
                m_vs = configuration->vertexShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_vs->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasPixelShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->pixelShader()->shaderFilePath()));
#endif
                m_ps = configuration->pixelShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_ps->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasHullShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->hullShader()->shaderFilePath()));
#endif
                m_hs = configuration->hullShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_hs->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasGeometryShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->geometryShader()->shaderFilePath()));
#endif
                m_gs = configuration->geometryShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_gs->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasDomainShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->domainShader()->shaderFilePath()));
#endif
                m_ds = configuration->domainShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_ds->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasRaygenerationShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->raygenerationShader()->shaderFilePath()));
#endif
                m_rg = configuration->raygenerationShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_rg->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasIntersectionShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->intersectionShader()->shaderFilePath()));
#endif
                m_is = configuration->intersectionShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_is->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasMissShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->missShader()->shaderFilePath()));
#endif
                m_ms = configuration->missShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_ms->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasAnyHitShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->anyHitShader()->shaderFilePath()));
#endif
                m_ah = configuration->anyHitShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_ah->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasClosestHitShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->closestHitShader()->shaderFilePath()));
#endif
                m_ch = configuration->closestHitShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_ch->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasAmplificationShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->amplificationShader()->shaderFilePath()));
#endif
                m_amp = configuration->amplificationShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_amp->registerForChange(this, [this]() { this->m_onChange(); });
            }
            if (configuration->hasMeshShader())
            {
#ifdef PRINT_SIGNATURES_DESCRIPTORS
                printRootSignatureFile(rsPathFromShaderPath(configuration->meshShader()->shaderFilePath()));
#endif
                m_mesh = configuration->meshShader()->load(m_device, m_storage, GraphicsApi::DX12);
                m_mesh->registerForChange(this, [this]() { this->m_onChange(); });
            }
        }
#endif

#if 1
#ifdef PRINT_SIGNATURES_DESCRIPTORS
        engine::string rangeTypeToString(D3D12_DESCRIPTOR_RANGE_TYPE type)
        {
            switch (type)
            {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: { return engine::string("D3D12_DESCRIPTOR_RANGE_TYPE_SRV"); }
            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: { return engine::string("D3D12_DESCRIPTOR_RANGE_TYPE_UAV"); }
            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: { return engine::string("D3D12_DESCRIPTOR_RANGE_TYPE_CBV"); }
            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: { return engine::string("D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER"); }
            }
            return "";
        }

        engine::string rangeVisibilityToString(D3D12_SHADER_VISIBILITY visibility)
        {
            switch (visibility)
            {
            case D3D12_SHADER_VISIBILITY_ALL: return "D3D12_SHADER_VISIBILITY_ALL";
            case D3D12_SHADER_VISIBILITY_VERTEX: return "D3D12_SHADER_VISIBILITY_VERTEX";
            case D3D12_SHADER_VISIBILITY_HULL: return "D3D12_SHADER_VISIBILITY_HULL";
            case D3D12_SHADER_VISIBILITY_DOMAIN: return "D3D12_SHADER_VISIBILITY_DOMAIN";
            case D3D12_SHADER_VISIBILITY_GEOMETRY: return "D3D12_SHADER_VISIBILITY_GEOMETRY";
            case D3D12_SHADER_VISIBILITY_PIXEL: return "D3D12_SHADER_VISIBILITY_PIXEL";
            case D3D12_SHADER_VISIBILITY_AMPLIFICATION: return "D3D12_SHADER_VISIBILITY_AMPLIFICATION";
            case D3D12_SHADER_VISIBILITY_MESH: return "D3D12_SHADER_VISIBILITY_MESH";
            }
            return "";
        }
#endif

        PipelineRootSignatureDX12::PipelineRootSignatureDX12(
            Device & device,
            engine::vector<ShaderContainerDX12>& shaders)
            : m_signature{}
            , m_bindingCounts{}
            , m_rootConstantCount{ 0 }
        {
            struct SrvUavCounts
            {
                size_t srvUavCount;
                size_t srvUavBindlessCount;
            };
            auto samplerCount = [](const shaders::Shader* shader)->size_t { return shader->samplerCount(); };
            auto srvCount = [](const shaders::Shader* shader)->SrvUavCounts
            {
                SrvUavCounts count{ 0, 0 };
                for (auto&& srv : shader->srvBindings())
                {
                    if (srv.type == shaders::BindingType::SRVBuffer ||
                        srv.type == shaders::BindingType::SRVTexture)
                        ++count.srvUavCount;
                    else if (srv.type == shaders::BindingType::BindlessSRVBuffer ||
                        srv.type == shaders::BindingType::BindlessSRVTexture)
                        ++count.srvUavBindlessCount;
                }
                return count;
            };
            auto uavCount = [](const shaders::Shader* shader)->SrvUavCounts
            {
                SrvUavCounts count{ 0, 0 };
                for (auto&& uav : shader->uavBindings())
                {
                    if (uav.type == shaders::BindingType::UAVBuffer ||
                        uav.type == shaders::BindingType::UAVTexture)
                        ++count.srvUavCount;
                    else if (uav.type == shaders::BindingType::BindlessUAVBuffer ||
                        uav.type == shaders::BindingType::BindlessUAVTexture)
                        ++count.srvUavBindlessCount;
                }
                return count;
            };
            auto rootConstantCount = [](const shaders::Shader* shader)->size_t { return shader->root_constants().size(); };
            auto constantCount = [](const shaders::Shader* shader)->size_t { return const_cast<shaders::Shader*>(shader)->constants().size(); };

            auto shaderParameterCount = [&](const shaders::Shader* shader)->ShaderBindingCountDX12
            {
                ShaderBindingCountDX12 counts{ 0, 0, 0, 0 };

                auto count = samplerCount(shader);
                counts.samplerBindingCount += count;
                count = count > 0 ? 1 : 0;
                counts.rootParameterCount += count;
                counts.descriptorTableCount += count;

                auto srvCounts = srvCount(shader);
                counts.resourceBindingCount += srvCounts.srvUavCount;
                auto parameterTablesSrv = (srvCounts.srvUavCount > 0 ? 1 : 0) + srvCounts.srvUavBindlessCount;
                counts.rootParameterCount += parameterTablesSrv;
                counts.descriptorTableCount += parameterTablesSrv;

                auto uavCounts = uavCount(shader);
                counts.resourceBindingCount += uavCounts.srvUavCount;
                auto parameterTablesUav = (uavCounts.srvUavCount > 0 ? 1 : 0) + uavCounts.srvUavBindlessCount;
                counts.rootParameterCount += parameterTablesUav;
                counts.descriptorTableCount += parameterTablesUav;

                count = rootConstantCount(shader) > 0 ? 1 : 0;
                m_rootConstantCount += count;
                counts.rootParameterCount += count;

                count = constantCount(shader);
                counts.resourceBindingCount += count;
                count = count > 0 ? 1 : 0;
                counts.rootParameterCount += count;
                counts.descriptorTableCount += count;

                return counts;
            };

            auto bindingCounts = [&]()->ShaderBindingCountDX12
            {
                ShaderBindingCountDX12 counts{ 0, 0, 0, 0 };
                for (auto&& shader : shaders)
                {
                    auto count = shaderParameterCount(shader.shader);
                    counts.resourceBindingCount += count.resourceBindingCount;
                    counts.samplerBindingCount += count.samplerBindingCount;
                    counts.rootParameterCount += count.rootParameterCount;
                    counts.descriptorTableCount += count.descriptorTableCount;
                }
                return counts;
            };

            m_bindingCounts = bindingCounts();
            engine::vector<D3D12_ROOT_PARAMETER1> parameters(m_bindingCounts.rootParameterCount);
            engine::vector<D3D12_DESCRIPTOR_RANGE1> descriptorTables(m_bindingCounts.descriptorTableCount);

            auto addEntry = [&](ShaderBindingCountDX12& entryId, size_t count, D3D12_DESCRIPTOR_RANGE_TYPE type, D3D12_SHADER_VISIBILITY visibility, UINT baseShaderRegister, UINT registerSpace)
            {
                descriptorTables[entryId.descriptorTableCount].RangeType = type;
                descriptorTables[entryId.descriptorTableCount].NumDescriptors = static_cast<UINT>(count);
                descriptorTables[entryId.descriptorTableCount].BaseShaderRegister = baseShaderRegister;
                descriptorTables[entryId.descriptorTableCount].RegisterSpace = registerSpace;
                descriptorTables[entryId.descriptorTableCount].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                descriptorTables[entryId.descriptorTableCount].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                LOG("Adding descriptor table[%i] RangeType: %s, NumDescriptors: %i, Visibility: %s", 
                    static_cast<int>(entryId.descriptorTableCount),
                    rangeTypeToString(type).c_str(),
                    static_cast<int>(count),
                    rangeVisibilityToString(visibility).c_str());
#endif

                parameters[entryId.rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                parameters[entryId.rootParameterCount].DescriptorTable.NumDescriptorRanges = 1;
                parameters[entryId.rootParameterCount].DescriptorTable.pDescriptorRanges = &descriptorTables[entryId.descriptorTableCount];
                parameters[entryId.rootParameterCount].ShaderVisibility = visibility;

                ++entryId.descriptorTableCount;
                ++entryId.rootParameterCount;
            };

            ShaderBindingCountDX12 indexes{ 0, 0, 0, 0 };
            for (auto&& shader : shaders)
            {
                auto rootConstCount = rootConstantCount(shader.shader);
                if (rootConstCount > 0)
                {
                    parameters[indexes.rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    parameters[indexes.rootParameterCount].Constants.ShaderRegister = 0;
                    parameters[indexes.rootParameterCount].Constants.RegisterSpace = 0;
                    parameters[indexes.rootParameterCount].Constants.Num32BitValues = 4;
                    parameters[indexes.rootParameterCount].ShaderVisibility = shader.visibility;
                    ++indexes.rootParameterCount;
                }

                auto sampCount = samplerCount(shader.shader);
                if (sampCount > 0) { addEntry(indexes, sampCount, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, shader.visibility, 0, shader.shader->setStartIndex()); }

                auto srCount = srvCount(shader.shader);
                if (srCount.srvUavCount > 0) { addEntry(indexes, srCount.srvUavCount, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shader.visibility, 0, shader.shader->setStartIndex()); }
                for(int i = 0; i < srCount.srvUavBindlessCount; ++i)
                    addEntry(indexes, static_cast<size_t>(-1), D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                        shader.visibility, 0, shader.shader->setStartIndex() + 1 + i);

                auto uaCount = uavCount(shader.shader);
                if (uaCount.srvUavCount > 0) { addEntry(indexes, uaCount.srvUavCount, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, shader.visibility, 0, shader.shader->setStartIndex()); }
                for (int i = 0; i < uaCount.srvUavBindlessCount; ++i)
                    addEntry(indexes, static_cast<size_t>(-1), D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shader.visibility, 0, shader.shader->setStartIndex() + 1 + i);

                auto constCount = constantCount(shader.shader);
                if (constCount > 0)
                {
                    addEntry(
                        indexes, 
                        constCount, 
                        D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 
                        shader.visibility, 
                        // constant buffer descriptor table might get 
                        // higher than b0 base shader register is if the shader
                        // has root constants. those go to b0
                        static_cast<UINT>(shader.shader->root_constants().size()), shader.shader->setStartIndex());
                }

            }

            D3D12_ROOT_SIGNATURE_DESC1 rootDesc;
            rootDesc.NumParameters = static_cast<UINT>(parameters.size());
            rootDesc.pParameters = (const D3D12_ROOT_PARAMETER1*)parameters.data();
            rootDesc.NumStaticSamplers = static_cast<UINT>(0);
            rootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)nullptr;
            rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            tools::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
            D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc;
            vdesc.Desc_1_1 = rootDesc;
            vdesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

            auto serializeRes = D3D12SerializeVersionedRootSignature(&vdesc, pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
            //ASSERT(SUCCEEDED(serializeRes));
            if (!SUCCEEDED(serializeRes))
            {
                engine::string temp;
            }

            auto createRootSignatureRes = static_cast<DeviceImplDX12*>(device.native())->device()->CreateRootSignature(
                1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
                DARKNESS_IID_PPV_ARGS(m_signature.GetAddressOf()));
            ASSERT(SUCCEEDED(createRootSignatureRes));
        }
#endif

#if 1
        PipelineStateDX12::PipelineStateDX12()
            : m_pipelineStateDesc{ engine::make_shared<D3D12_GRAPHICS_PIPELINE_STATE_DESC>() }
            , m_computePipelineStateDesc{ engine::make_shared<D3D12_COMPUTE_PIPELINE_STATE_DESC>() }
            , m_finalized{ false }
            , m_inErrorState{ false }
        {
            initialize();
        }

        uint64_t PipelineStateDX12::hash(uint64_t hash) const
        {
            uint64_t* rasterizerStatePtr = reinterpret_cast<uint64_t*>(&m_pipelineStateDesc->RasterizerState);
            int qw = sizeof(D3D12_RASTERIZER_DESC) / 8;
            int leftBytes = sizeof(D3D12_RASTERIZER_DESC) - (qw * 8);
            for (int i = 0; i < qw; ++i)
            {
                hash = shaders::fnv1aHash(*rasterizerStatePtr, hash);
                ++rasterizerStatePtr;
            }
            if(leftBytes == 1)
                hash = shaders::fnv1aHash(*reinterpret_cast<uint8_t*>(rasterizerStatePtr), hash);
            else if (leftBytes == 2)
                hash = shaders::fnv1aHash(*reinterpret_cast<uint16_t*>(rasterizerStatePtr), hash);
            else if (leftBytes == 4)
                hash = shaders::fnv1aHash(*reinterpret_cast<uint32_t*>(rasterizerStatePtr), hash);
            
            return hash;
        }

        void PipelineStateDX12::setBlendState(const BlendDescription& desc)
        {
            m_pipelineStateDesc->BlendState = dxBlendDesc(desc);
        }

        void PipelineStateDX12::setRasterizerState(const RasterizerDescription& desc)
        {
            m_pipelineStateDesc->RasterizerState = dxRasterizerDesc(desc);
        }

        void PipelineStateDX12::setDepthStencilState(const DepthStencilDescription& desc)
        {
            m_pipelineStateDesc->DepthStencilState = dxDepthStencilDesc(desc);
        }

        void PipelineStateDX12::setSampleMask(unsigned int mask)
        {
            m_pipelineStateDesc->SampleMask = mask;
        }

        void PipelineStateDX12::setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency)
        {
            m_pipelineStateDesc->PrimitiveTopologyType = dxPrimitiveTopologyType(dxPrimitiveTopologyType(type, adjacency));
            topology = dxPrimitiveTopologyType(type, adjacency);
        }

        void PipelineStateDX12::setPrimitiveRestart(IndexBufferStripCutValue value)
        {
            m_pipelineStateDesc->IBStripCutValue = dxIndexBufferStripCutValue(value);
        }

        void PipelineStateDX12::setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            setRenderTargetFormats({ RTVFormat }, DSVFormat, msaaCount, msaaQuality);
        }

        void PipelineStateDX12::setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            // Null format array conflicts with non-zero length
            uint32_t index = 0;
            for (auto&& format : RTVFormats)
            {
                m_pipelineStateDesc->RTVFormats[index] = dxFormat(format);
                ++index;
            }
            m_pipelineStateDesc->NumRenderTargets = static_cast<UINT>(RTVFormats.size());
            m_pipelineStateDesc->DSVFormat = dxFormat(DSVFormat);
            m_pipelineStateDesc->SampleDesc.Count = msaaCount;
            m_pipelineStateDesc->SampleDesc.Quality = msaaQuality;

            typedef enum D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS {
                D3D11_STANDARD_MULTISAMPLE_PATTERN = 0xffffffff,
                D3D11_CENTER_MULTISAMPLE_PATTERN = 0xfffffffe
            } D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS;

            m_pipelineStateDesc->SampleDesc.Quality = msaaCount > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
        }

        void PipelineStateDX12::initialize()
        {
            m_pipelineStateDesc->BlendState.AlphaToCoverageEnable = false;
            m_pipelineStateDesc->BlendState.IndependentBlendEnable = false;
            m_pipelineStateDesc->BlendState.RenderTarget[0].BlendEnable = false;
            m_pipelineStateDesc->BlendState.RenderTarget[0].LogicOpEnable = false;
            m_pipelineStateDesc->BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
            m_pipelineStateDesc->BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
            m_pipelineStateDesc->BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            m_pipelineStateDesc->BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            m_pipelineStateDesc->BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            m_pipelineStateDesc->BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            m_pipelineStateDesc->BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
            m_pipelineStateDesc->BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            m_pipelineStateDesc->SampleMask = UINT_MAX;
            m_pipelineStateDesc->RasterizerState = dxRasterizerDesc(RasterizerDescription());
            m_pipelineStateDesc->PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }

        void PipelineStateDX12::finalize(
            Device& device, 
            const PipelineShadersDX12& shaders, 
            PipelineRootSignatureDX12& rootSignature, 
            engine::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayouts,
            FinalizeFlags flags)
        {
            if (shaders.cs()) m_computePipelineStateDesc->CS = static_cast<const ShaderBinaryImplDX12*>(shaders.cs()->native())->native();
            if (shaders.vs()) m_pipelineStateDesc->VS = static_cast<const ShaderBinaryImplDX12*>(shaders.vs()->native())->native();
            if (shaders.ps()) m_pipelineStateDesc->PS = static_cast<const ShaderBinaryImplDX12*>(shaders.ps()->native())->native();
            if (shaders.gs()) m_pipelineStateDesc->GS = static_cast<const ShaderBinaryImplDX12*>(shaders.gs()->native())->native();
            if (shaders.ds()) m_pipelineStateDesc->DS = static_cast<const ShaderBinaryImplDX12*>(shaders.ds()->native())->native();
            if (shaders.hs()) m_pipelineStateDesc->HS = static_cast<const ShaderBinaryImplDX12*>(shaders.hs()->native())->native();

            m_pipelineStateDesc->InputLayout.NumElements = static_cast<UINT>(inputLayouts.size());
            m_pipelineStateDesc->InputLayout.pInputElementDescs = inputLayouts.data();

            if (!shaders.cs())
                m_pipelineStateDesc->pRootSignature = rootSignature.signature();
            else
                m_computePipelineStateDesc->pRootSignature = rootSignature.signature();

            if(!m_finalized || flags == FinalizeFlags::ForceReCreate)
            {
                if (m_finalized)
                    m_pipeline = {};

                if (!shaders.cs())
                {
                    auto res = static_cast<DeviceImplDX12*>(device.native())->device()->CreateGraphicsPipelineState(
                        m_pipelineStateDesc.get(),
                        DARKNESS_IID_PPV_ARGS(m_pipeline.GetAddressOf()));
                    if (SUCCEEDED(res))
                    {
                        m_inErrorState = false;
                    }
                    else
                    {
                        m_inErrorState = true;
                    }
                }
                else
                {
                    auto res = static_cast<DeviceImplDX12*>(device.native())->device()->CreateComputePipelineState(
                        m_computePipelineStateDesc.get(),
                        DARKNESS_IID_PPV_ARGS(m_pipeline.GetAddressOf()));
                    if (SUCCEEDED(res))
                    {
                        m_inErrorState = false;
                    }
                    else
                    {
                        m_inErrorState = true;
                    }
                }
                m_finalized = true;
            }
        }
#endif

        DescriptorTablesDX12::DescriptorTablesDX12(
            Device& device,
            size_t shaderResourceBindingCount,
            size_t samplerBindingCount)
            : m_resourceHandles{ static_cast<DeviceImplDX12*>(device.native())->heaps().shaderVisible_cbv_srv_uav->getDescriptor(shaderResourceBindingCount) }
            , m_samplerHandles{ static_cast<DeviceImplDX12*>(device.native())->heaps().shaderVisible_sampler->getDescriptor(samplerBindingCount) }
        {}

        DescriptorHandleDX12& DescriptorTablesDX12::resourceHandles()
        {
            return m_resourceHandles;
        };
        DescriptorHandleDX12& DescriptorTablesDX12::samplerHandles()
        {
            return m_samplerHandles;
        };

        PipelineImplDX12::PipelineImplDX12(
            Device& device,
            ShaderStorage& storage)
            : m_device{ device }
            , m_storage{ storage }
            , m_pipelineShaders{ nullptr }
            , m_rootSignature{ nullptr }
            , m_pipelineState{ }
            , m_inputLayouts{}
            , m_semanticNames(4096)
            , m_semanticIndex{ 0 }
            , m_lastTableBindings{ nullptr }
            , m_hashResourceStorage{}
        {
        }

        PipelineImplDX12::~PipelineImplDX12()
        {
            
        }

        void PipelineImplDX12::onShaderChange()
        {
            m_rootSignature = engine::make_unique<PipelineRootSignatureDX12>(m_device, m_shaders);

            createInputLayout();
            m_pipelineState.finalize(m_device, *m_pipelineShaders, *m_rootSignature, m_inputLayouts, PipelineStateDX12::FinalizeFlags::ForceReCreate);
            
            m_hashResourceStorage.clear();

        }

#if 1
        void PipelineImplDX12::setBlendState(const BlendDescription& desc)
        {
            m_pipelineState.setBlendState(desc);
        }

        void PipelineImplDX12::setRasterizerState(const RasterizerDescription& desc)
        {
            m_pipelineState.setRasterizerState(desc);
        }

        void PipelineImplDX12::setDepthStencilState(const DepthStencilDescription& desc)
        {
            m_pipelineState.setDepthStencilState(desc);
        }

        void PipelineImplDX12::setSampleMask(unsigned int mask)
        {
            m_pipelineState.setSampleMask(mask);
        }

        void PipelineImplDX12::setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency)
        {
            m_pipelineState.setPrimitiveTopologyType(type, adjacency);
        }

        void PipelineImplDX12::setPrimitiveRestart(IndexBufferStripCutValue value)
        {
            m_pipelineState.setPrimitiveRestart(value);
        }

        void PipelineImplDX12::setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            m_pipelineState.setRenderTargetFormat(RTVFormat, DSVFormat, msaaCount, msaaQuality);
        }

        void PipelineImplDX12::setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            m_pipelineState.setRenderTargetFormats(RTVFormats, DSVFormat, msaaCount, msaaQuality);
        }
#endif

        void PipelineImplDX12::createInputLayout()
        {
            auto shaderTypeToDXGI = [](const engine::string& type)->DXGI_FORMAT
            {
                if (type == "uint") return DXGI_FORMAT_R32_UINT;
                else if (type == "uint2") return DXGI_FORMAT_R32G32_UINT;
                else if (type == "uint3") return DXGI_FORMAT_R32G32B32_UINT;
                else if (type == "uint4") return DXGI_FORMAT_R32G32B32A32_UINT;
                else if (type == "float") return DXGI_FORMAT_R32_FLOAT;
                else if (type == "float2") return DXGI_FORMAT_R32G32_FLOAT;
                else if (type == "float3") return DXGI_FORMAT_R32G32B32_FLOAT;
                else if (type == "float4") return DXGI_FORMAT_R32G32B32A32_FLOAT;
                else if (type == "bool") return DXGI_FORMAT_R32_UINT;
                else
                    ASSERT(false, "Unhandled shader input type");
                return DXGI_FORMAT_UNKNOWN;
            };
            auto extractSemanticIndex = [](const engine::string& semanticName)->uint32_t
            {
                engine::string index = "";
                //for (auto&& ch : semanticName)
                for (auto ch = semanticName.rbegin(); ch != semanticName.rend(); ++ch)
                {
                    if ((*ch) >= 48 && (*ch) <= 57)
                    {
                        engine::string temp = "";
                        temp += *ch;
                        index.insert(0, temp);
                    }
                    else break;
                }
                if (index.size() > 0)
                    return static_cast<uint32_t>(std::stoi(index.c_str()));
                else return 0u;
            };
            auto extractSemanticName = [](const engine::string& semanticName)->engine::string
            {
                int indexLength = 0;
                for (auto ch = semanticName.rbegin(); ch != semanticName.rend(); ++ch)
                {
                    if ((*ch) >= 48 && (*ch) <= 57)
                    {
                        ++indexLength;
                    }
                    else break;
                }
                return semanticName.substr(0, semanticName.length() - indexLength);
            };

            auto addInputsToLayout = [shaderTypeToDXGI, extractSemanticIndex, extractSemanticName]
            (engine::vector<D3D12_INPUT_ELEMENT_DESC>& layouts,
                const engine::vector<shaders::ShaderInputParameter>& inputs,
                engine::vector<char>& semanticNames,
                uint32_t& semanticIndex)
            {
                for (size_t i = 0; i < inputs.size(); ++i)
                {
                    D3D12_INPUT_ELEMENT_DESC desc;
                    desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
                    desc.Format = shaderTypeToDXGI(inputs[i].type);
                    desc.InputSlot = 0;
                    desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    desc.InstanceDataStepRate = 0;
                    desc.SemanticIndex = extractSemanticIndex(inputs[i].semanticName);

                    auto semanticNameWithoutNumber = extractSemanticName(inputs[i].semanticName);
                    auto currentSemanticIndex = semanticIndex;
                    ASSERT(currentSemanticIndex + semanticNameWithoutNumber.size() < semanticNames.size(), "DX12 Pipeline ran out of semantics namespace. (Shader parameter input names are so long that we can't store them within 4kb)");
                    memcpy(&semanticNames[currentSemanticIndex], semanticNameWithoutNumber.data(), semanticNameWithoutNumber.size());
                    semanticIndex += static_cast<uint32_t>(semanticNameWithoutNumber.size()) + 1u;
                    desc.SemanticName = &semanticNames[currentSemanticIndex];

                    //printElementDesc(desc);

                    bool identicalSemanticFound = false;
                    for (auto&& l : layouts)
                    {
                        if ((l.SemanticIndex == desc.SemanticIndex) &&
                            (strcmp(l.SemanticName, desc.SemanticName) == 0))
                        {
                            identicalSemanticFound = true;
                            break;
                        }
                    }
                    if (!identicalSemanticFound)
                        layouts.emplace_back(desc);
                }
            };

            m_semanticIndex = 0;
            memset(&m_semanticNames[0], '\0', m_semanticNames.size());

            for(auto&& shader : m_shaders)
                addInputsToLayout(m_inputLayouts, shader.shader->inputParameters(shader.shader->currentPermutationId()), m_semanticNames, m_semanticIndex);

            m_pipelineState.pipelineStateDesc().InputLayout.NumElements = static_cast<UINT>(m_inputLayouts.size());
            m_pipelineState.pipelineStateDesc().InputLayout.pInputElementDescs = m_inputLayouts.data();
        }

        void PipelineImplDX12::configure(CommandListImplIf* cmdList, shaders::PipelineConfiguration* configuration)
        {
            auto& commandList = *static_cast<CommandListImplDX12*>(cmdList);
            if (m_shaders.size() == 0)
                gatherShaders(configuration);

            if (!m_pipelineShaders)
                m_pipelineShaders = engine::make_unique<PipelineShadersDX12>(m_device, m_storage, [this]() { this->onShaderChange(); }, configuration);

            if (!configuration->hasRaygenerationShader() &&
                !configuration->hasIntersectionShader() &&
                !configuration->hasMissShader() &&
                !configuration->hasAnyHitShader() &&
                !configuration->hasClosestHitShader())
            {
                if (!m_rootSignature)
                    m_rootSignature = engine::make_unique<PipelineRootSignatureDX12>(m_device, m_shaders);

                if (!m_pipelineState.finalized())
                {
                    createInputLayout();
                    m_pipelineState.finalize(m_device, *m_pipelineShaders, *m_rootSignature, m_inputLayouts);
                }
            }
            else
            {
#ifdef DXR_BUILD
                auto getShaderRootSignatureBinary = [](const shaders::Shader* shader)->engine::vector<char>
                {
                    engine::vector<char> result;
                    auto binaryPath = shader->shaderFilePath() + ".rso";
                    std::ifstream file(binaryPath, std::ios::in);
                    if (file.is_open())
                    {
                        auto begin = file.tellg();
                        file.seekg(0, std::ios::end);
                        auto end = file.tellg();
                        size_t size = static_cast<size_t>(end - begin);
                        result.resize(size);
                        file.seekg(0, std::ios::beg);
                        file.read(&result[0], end - begin);
                        file.close();
                    }
                    return result;
                };

                auto getShaderRootSignature = [&](const shaders::Shader* shader)->tools::ComPtr<ID3D12RootSignature>
                {
                    auto binary = getShaderRootSignatureBinary(shader);

                    tools::ComPtr<ID3D12RootSignature> pRootSig;
                    auto hr = DeviceImplGet::impl(m_device).dxrDevice()->CreateRootSignature(
                        0, 
                        reinterpret_cast<void*>(binary.data()),
                        binary.size(),
                        DARKNESS_IID_PPV_ARGS(pRootSig.GetAddressOf()));
                    ASSERT(SUCCEEDED(hr));
                    return pRootSig;
                };

                struct DxilLibraryDesc
                {
                    D3D12_EXPORT_DESC exportDesc;
                    D3D12_DXIL_LIBRARY_DESC desc;
                };
                auto createDXILLibraryDesc = [](const ShaderBinary* shaderBinary, const wchar_t* exportName)->engine::shared_ptr<DxilLibraryDesc>
                {
                    auto res = engine::make_shared<DxilLibraryDesc>();
                    res->exportDesc.Name = exportName;
                    res->exportDesc.ExportToRename = L"";
                    res->exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                    res->desc.DXILLibrary = ShaderBinaryImplGet::impl(*shaderBinary).native();
                    res->desc.NumExports = 1;
                    res->desc.pExports = &res->exportDesc;

                    return res;
                };
                
                auto createHitGroup = []()
                {
                    D3D12_HIT_GROUP_DESC desc;
                    desc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES; // D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE
                    desc.HitGroupExport = L"HitGroup";
                    desc.ClosestHitShaderImport = L"ClosestHit";
                    desc.AnyHitShaderImport = L"";
                    desc.IntersectionShaderImport = L"";
                    return desc;
                };

                // DXR pipeline
                auto rgSignature = getShaderRootSignature(configuration->raygenerationShader());
                auto msSignature = getShaderRootSignature(configuration->missShader());
                auto chSignature = getShaderRootSignature(configuration->closestHitShader());

                auto rgDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->rg(), L"RayGen");
                auto msDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->ms(), L"Miss");
                auto chDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->ch(), L"ClosestHit");

                auto hitGroup = createHitGroup();

                uint64_t subobjectCount =
                    3 +      // DXC Shader binary count
                    1 +      // Hit group count
                    1 +      // Shader configuration
                    1 +      // Shader payload
                    2 * 3 +  // Root signature declaration + association
                    2 +      // Empty global and local root signatures
                    1;
                uint32_t currentIndex = 0;

                engine::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);

                // shader binaries
                {
                    D3D12_STATE_SUBOBJECT subObject = {};
                    subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                    subObject.pDesc = &rgDxilLibDesc->desc;
                    subobjects[currentIndex++] = subObject;
                }
                {
                    D3D12_STATE_SUBOBJECT subObject = {};
                    subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                    subObject.pDesc = &msDxilLibDesc->desc;
                    subobjects[currentIndex++] = subObject;
                }
                {
                    D3D12_STATE_SUBOBJECT subObject = {};
                    subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
                    subObject.pDesc = &chDxilLibDesc->desc;
                    subobjects[currentIndex++] = subObject;
                }

                // hit groups
                {
                    D3D12_STATE_SUBOBJECT subObject = {};
                    subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
                    subObject.pDesc = &hitGroup;
                    subobjects[currentIndex++] = subObject;
                }

                // shader configuration
                {
                    D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
                    shaderDesc.MaxPayloadSizeInBytes = 4 * sizeof(float);   // RGB + distance
                    shaderDesc.MaxAttributeSizeInBytes = 2 * sizeof(float); // barycentric coordinates

                    D3D12_STATE_SUBOBJECT shaderConfigObject = {};
                    shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
                    shaderConfigObject.pDesc = &shaderDesc;

                    subobjects[currentIndex++] = shaderConfigObject;
                }

                // shader payload
                engine::vector<LPCWSTR> exportStrings(3);
                engine::vector<ID3D12RootSignature*> signatures(3);
                struct Association
                {
                    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association;
                    D3D12_STATE_SUBOBJECT object;
                };
                engine::vector<Association> associations(3);
                exportStrings[0] = L"RayGen";
                signatures[0] = rgSignature.Get();
                exportStrings[1] = L"Miss";
                signatures[1] = msSignature.Get();
                exportStrings[2] = L"HitGroup";
                signatures[2] = chSignature.Get();

                // Add a subobject for the association between shaders and the payload
                D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
                shaderPayloadAssociation.NumExports = static_cast<UINT>(exportStrings.size());
                shaderPayloadAssociation.pExports = exportStrings.data();

                // Associate the set of shaders with the payload defined in the previous subobject
                shaderPayloadAssociation.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

                // Create and store the payload association object
                D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
                shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
                shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;
                subobjects[currentIndex++] = shaderPayloadAssociationObject;


                // root signature declaration and association
                for(int i = 0; i < 3; ++i)
                {
                    // Add a subobject to declare the root signature
                    D3D12_STATE_SUBOBJECT rootSigObject = {};
                    rootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
                    rootSigObject.pDesc = signatures[i];

                    subobjects[currentIndex++] = rootSigObject;

                    // Add a subobject for the association between the exported shader symbols and the root
                    // signature
                    associations[i].association.NumExports = 1;
                    associations[i].association.pExports = &exportStrings[i];
                    associations[i].association.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

                    D3D12_STATE_SUBOBJECT rootSigAssociationObject = {};
                    rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
                    rootSigAssociationObject.pDesc = &associations[i].association;

                    subobjects[currentIndex++] = rootSigAssociationObject;
                }

                struct DummyRootSignatures
                {
                    tools::ComPtr<ID3D12RootSignature> global;
                    tools::ComPtr<ID3D12RootSignature> local;
                };
                DummyRootSignatures dummySignatures;

                auto createDummyRootSignatures = [&]()
                {
                    // Creation of the global root signature
                    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
                    rootDesc.NumParameters = 0;
                    rootDesc.pParameters = nullptr;
                    // A global root signature is the default, hence this flag
                    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

                    HRESULT hr = 0;

                    ID3DBlob* serializedRootSignature;
                    ID3DBlob* error;

                    // Create the empty global root signature
                    hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                    &serializedRootSignature, &error);
                    if (FAILED(hr))
                    {
                        throw std::logic_error("Could not serialize the global root signature");
                    }
                    hr = DeviceImplGet::impl(m_device).dxrDevice()->CreateRootSignature(
                        0, 
                        serializedRootSignature->GetBufferPointer(),
                        serializedRootSignature->GetBufferSize(),
                        DARKNESS_IID_PPV_ARGS(dummySignatures.global.GetAddressOf()));

                    serializedRootSignature->Release();
                    if (FAILED(hr))
                    {
                        throw std::logic_error("Could not create the global root signature");
                    }

                    // Create the local root signature, reusing the same descriptor but altering the creation flag
                    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
                    hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                    &serializedRootSignature, &error);
                    if (FAILED(hr))
                    {
                        throw std::logic_error("Could not serialize the local root signature");
                    }
                    hr = DeviceImplGet::impl(m_device).dxrDevice()->CreateRootSignature(
                        0, 
                        serializedRootSignature->GetBufferPointer(),
                        serializedRootSignature->GetBufferSize(),
                        DARKNESS_IID_PPV_ARGS(dummySignatures.local.GetAddressOf()));

                    serializedRootSignature->Release();
                    if (FAILED(hr))
                    {
                    throw std::logic_error("Could not create the local root signature");
                    }
                };
                createDummyRootSignatures();


                // The pipeline construction always requires an empty global root signature
                D3D12_STATE_SUBOBJECT globalRootSig;
                globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
                ID3D12RootSignature* dgSig = dummySignatures.global.Get();
                globalRootSig.pDesc = &dgSig;

                subobjects[currentIndex++] = globalRootSig;

                // The pipeline construction always requires an empty local root signature
                D3D12_STATE_SUBOBJECT dummyLocalRootSig;
                dummyLocalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
                ID3D12RootSignature* dlSig = dummySignatures.local.Get();
                dummyLocalRootSig.pDesc = &dlSig;
                subobjects[currentIndex++] = dummyLocalRootSig;

                // Add a subobject for the ray tracing pipeline configuration
                D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
                pipelineConfig.MaxTraceRecursionDepth = 1;

                D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
                pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
                pipelineConfigObject.pDesc = &pipelineConfig;

                subobjects[currentIndex++] = pipelineConfigObject;

                // Describe the ray tracing pipeline state object
                D3D12_STATE_OBJECT_DESC pipelineDesc = {};
                pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
                pipelineDesc.NumSubobjects = currentIndex; // static_cast<UINT>(subobjects.size());
                pipelineDesc.pSubobjects = subobjects.data();

                tools::ComPtr<ID3D12StateObject> rtStateObject;

                // Create the state object
                HRESULT hr = DeviceImplGet::impl(m_device).dxrDevice()->CreateStateObject(&pipelineDesc, DARKNESS_IID_PPV_ARGS(rtStateObject.GetAddressOf()));
                ASSERT(SUCCEEDED(hr));
#endif

#if 0
                auto getShaderSourcePath = [](const shaders::Shader* shader)->engine::string
                {
                    // binaryPath =     "C:/work/darkness/darkness-engine/data/shaders/dx12/core/dxr/RayTraceTest.rg"
                    // sourcePathRoot = "C:/work/darkness/darkness-engine"
                    // sourcePath =     "C:/work/darkness/darkness-engine/shaders/          core/dxr/RayTraceTest.rg.hlsl"
                    auto binaryPath = shader->shaderFilePath();
                    auto searchStr = engine::string("/data/shaders");
                    auto platform = engine::string("/dx12/");
                    auto sourcePathRoot = binaryPath.substr(0, binaryPath.find(searchStr));
                    auto sourceBegin = sourcePathRoot.length() + searchStr.length() + platform.length();
                    auto sourcePathEnd = binaryPath.substr(
                        sourceBegin,
                        binaryPath.length() - sourceBegin);

                    auto sourcePath = sourcePathRoot + "/shaders/" + sourcePathEnd + ".hlsl";
                    return sourcePath;
                };

                auto createRayGenSignature = [&]()->ComPtr<ID3D12RootSignature>
                {
                    nv_helpers_dx12::RootSignatureGenerator rsc;
                    rsc.AddHeapRangesParameter(
                        { {0 /*u0*/, 1 /*1 descriptor */, 0 /*use the implicit register space 0*/,
                            D3D12_DESCRIPTOR_RANGE_TYPE_UAV /* UAV representing the output buffer*/,
                            0 /*heap slot where the UAV is defined*/},
                            {0 /*t0*/, 1, 0,
                            D3D12_DESCRIPTOR_RANGE_TYPE_SRV /*Top-level acceleration structure*/,
                            1} });

                    auto rs = rsc.Generate(DeviceImplGet::impl(m_device).dxrDevice(), true);
                    ComPtr<ID3D12RootSignature> res;
                    res.Attach(rs);
                    return res;
                };

                auto createHitSignature = [&]()->ComPtr<ID3D12RootSignature>
                {
                    nv_helpers_dx12::RootSignatureGenerator rsc;
                    auto rs = rsc.Generate(DeviceImplGet::impl(m_device).dxrDevice(), true);
                    ComPtr<ID3D12RootSignature> res;
                    res.Attach(rs);
                    return res;
                };

                auto createMissSignature = [&]()->ComPtr<ID3D12RootSignature>
                {
                    nv_helpers_dx12::RootSignatureGenerator rsc;
                    auto rs = rsc.Generate(DeviceImplGet::impl(m_device).dxrDevice(), true);
                    ComPtr<ID3D12RootSignature> res;
                    res.Attach(rs);
                    return res;
                };

                struct DxilLibraryDesc
                {
                    D3D12_EXPORT_DESC exportDesc;
                    D3D12_DXIL_LIBRARY_DESC desc;
                };
                auto createDXILLibraryDesc = [](const ShaderBinary* shaderBinary, const wchar_t* exportName)->engine::shared_ptr<DxilLibraryDesc>
                {
                    auto res = engine::make_shared<DxilLibraryDesc>();
                    res->exportDesc.Name = exportName;
                    res->exportDesc.ExportToRename = nullptr;
                    res->exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

                    res->desc.DXILLibrary = ShaderBinaryImplGet::impl(*shaderBinary).native();
                    res->desc.NumExports = 1;
                    res->desc.pExports = &res->exportDesc;

                    return res;
                };

                auto rgDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->rg(), L"RayGen");
                auto msDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->ms(), L"Miss");
                auto chDxilLibDesc = createDXILLibraryDesc(m_pipelineShaders->ch(), L"ClosestHit");

                nv_helpers_dx12::RayTracingPipelineGenerator pipeline(
                    DeviceImplGet::impl(m_device).device(),
                    DeviceImplGet::impl(m_device).dxrDevice());

                auto rgSrcPath = getShaderSourcePath(configuration->raygenerationShader());
                auto wrgSrcPath = toWideString(rgSrcPath);

                auto msSrcPath = getShaderSourcePath(configuration->missShader());
                auto wmsSrcPath = toWideString(msSrcPath);

                auto chSrcPath = getShaderSourcePath(configuration->closestHitShader());
                auto wchSrcPath = toWideString(chSrcPath);

                auto m_rayGenLibrary = nv_helpers_dx12::CompileShaderLibrary(wrgSrcPath.c_str());
                auto m_missLibrary = nv_helpers_dx12::CompileShaderLibrary(wmsSrcPath.c_str());
                auto m_hitLibrary = nv_helpers_dx12::CompileShaderLibrary(wchSrcPath.c_str());

                pipeline.AddLibrary(m_rayGenLibrary, { L"RayGen" });
                pipeline.AddLibrary(m_missLibrary, { L"Miss" });
                pipeline.AddLibrary(m_hitLibrary, { L"ClosestHit" });

                /*auto m_rayGenSignature = createRayGenSignature();
                auto m_missSignature = createMissSignature();
                auto m_hitSignature = createHitSignature();*/

                auto getShaderRootSignatureBinary = [](const shaders::Shader* shader)->engine::vector<char>
                {
                    engine::vector<char> result;
                    auto binaryPath = shader->shaderFilePath() + ".rso";
                    std::ifstream file(binaryPath, std::ios::in);
                    if (file.is_open())
                    {
                        auto begin = file.tellg();
                        file.seekg(0, std::ios::end);
                        auto end = file.tellg();
                        size_t size = static_cast<size_t>(end - begin);
                        result.resize(size);
                        file.seekg(0, std::ios::beg);
                        file.read(&result[0], end - begin);
                        file.close();
                    }
                    return result;
                };

                auto getShaderRootSignature = [&](const shaders::Shader* shader)->tools::ComPtr<ID3D12RootSignature>
                {
                    auto binary = getShaderRootSignatureBinary(shader);

                    tools::ComPtr<ID3D12RootSignature> pRootSig;
                    auto hr = DeviceImplGet::impl(m_device).dxrDevice()->CreateRootSignature(
                        0,
                        reinterpret_cast<void*>(binary.data()),
                        binary.size(),
                        DARKNESS_IID_PPV_ARGS(pRootSig.GetAddressOf()));
                    ASSERT(SUCCEEDED(hr));
                    return pRootSig;
                };

                auto rgSignature = getShaderRootSignature(configuration->raygenerationShader());
                auto msSignature = getShaderRootSignature(configuration->missShader());
                auto chSignature = getShaderRootSignature(configuration->closestHitShader());

                pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");

                pipeline.AddRootSignatureAssociation(rgSignature.Get(), { L"RayGen" });
                pipeline.AddRootSignatureAssociation(msSignature.Get(), { L"Miss" });
                pipeline.AddRootSignatureAssociation(chSignature.Get(), { L"HitGroup" });

                pipeline.SetMaxPayloadSize(4 * sizeof(float)); // RGB + distance
                pipeline.SetMaxAttributeSize(2 * sizeof(float)); // barycentric coordinates
                pipeline.SetMaxRecursionDepth(1);
                auto m_rtStateObject = pipeline.Generate({
                    &rgDxilLibDesc->desc,
                    &msDxilLibDesc->desc,
                    &chDxilLibDesc->desc
                    });

                ComPtr<ID3D12StateObjectProperties> m_rtStateObjectProps;
                auto res = m_rtStateObject->QueryInterface(DARKNESS_IID_PPV_ARGS(m_rtStateObjectProps.GetAddressOf()));



                nv_helpers_dx12::ShaderBindingTableGenerator m_sbtHelper;
                m_sbtHelper.Reset();
                
                DescriptorTables desc(m_device, 2, 0);

                m_sbtHelper.AddRayGenerationProgram(L"RayGen", { reinterpret_cast<void*>(desc.resourceHandles().gpuHandle().ptr) });
                m_sbtHelper.AddMissProgram(L"Miss", {});
                m_sbtHelper.AddHitGroup(L"HitGroup", {});

                uint32_t sbtSize = m_sbtHelper.ComputeSBTSize(DeviceImplGet::impl(m_device).dxrDevice());
                auto m_sbtStorage = m_device.createBuffer(BufferDescription()
                    .name("sbtStorage")
                    .usage(ResourceUsage::Upload)
                    .elementSize(1)
                    .elements(sbtSize));

                m_sbtHelper.Generate(m_sbtStorage.resource().m_impl->native(), m_rtStateObjectProps.Get());
#endif
            }

            auto pipelineHash = configuration->hash();
            pipelineHash = m_pipelineState.hash(pipelineHash);

            eraseUnused();

            auto storedPipeline = m_hashResourceStorage.find(pipelineHash);
            if (storedPipeline != m_hashResourceStorage.end())
            {
                writeResources(commandList, storedPipeline->second, WriteFlags::UpdateConstants);
                m_lastTableBindings = &storedPipeline->second.tableBindings;
                storedPipeline->second.lastUsedFrame = m_device.frameNumber();
            }
            else
            {
                PipelineCache newPipeline{
                    engine::make_unique<DescriptorTablesDX12>(
                        m_device,
                        m_rootSignature->bindingCounts().resourceBindingCount,
                        m_rootSignature->bindingCounts().samplerBindingCount)
                };
                writeResources(commandList, newPipeline, WriteFlags::WriteDescriptors);
                auto temp = m_hashResourceStorage.insert(std::pair<uint64_t, PipelineCache>{ pipelineHash, std::move(newPipeline) });
                m_lastTableBindings = &temp.first->second.tableBindings;
                temp.first->second.lastUsedFrame = m_device.frameNumber();
            }
        }

        bool PipelineImplDX12::valid() const
        {
            return !m_pipelineState.errorState() && m_pipelineState.finalized();
        }

        void PipelineImplDX12::gatherShaders(shaders::PipelineConfiguration* configuration)
        {
            if (configuration->hasComputeShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->computeShader(), D3D12_SHADER_VISIBILITY_ALL });
            if (configuration->hasVertexShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->vertexShader(), D3D12_SHADER_VISIBILITY_VERTEX });
            if (configuration->hasPixelShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->pixelShader(), D3D12_SHADER_VISIBILITY_PIXEL });
            if (configuration->hasGeometryShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->geometryShader(), D3D12_SHADER_VISIBILITY_GEOMETRY });
            if (configuration->hasDomainShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->domainShader(), D3D12_SHADER_VISIBILITY_DOMAIN });
            if (configuration->hasHullShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->hullShader(), D3D12_SHADER_VISIBILITY_HULL });

            if (configuration->hasRaygenerationShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->raygenerationShader(), D3D12_SHADER_VISIBILITY_ALL });
            if (configuration->hasIntersectionShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->intersectionShader(), D3D12_SHADER_VISIBILITY_ALL });
            if (configuration->hasMissShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->missShader(), D3D12_SHADER_VISIBILITY_ALL });
            if (configuration->hasAnyHitShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->anyHitShader(), D3D12_SHADER_VISIBILITY_ALL });
            if (configuration->hasClosestHitShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->closestHitShader(), D3D12_SHADER_VISIBILITY_ALL });

#ifndef _DURANGO
            if (configuration->hasAmplificationShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->amplificationShader(), D3D12_SHADER_VISIBILITY_AMPLIFICATION });
            if (configuration->hasMeshShader()) m_shaders.emplace_back(ShaderContainerDX12{ configuration->meshShader(), D3D12_SHADER_VISIBILITY_MESH });
#endif
        }

        void PipelineImplDX12::eraseUnused()
        {
            auto currentFrame = m_device.frameNumber();

            auto deadCache = currentFrame;
            if (deadCache > BackBufferCount)
                deadCache -= BackBufferCount;
            else
                deadCache = 0;

            bool erased = true;
            while (erased)
            {
                erased = false;
                for (auto&& cached : m_hashResourceStorage)
                {
                    if (cached.second.lastUsedFrame < deadCache)
                    {
                        m_hashResourceStorage.erase(cached.first);
                        erased = true;
                        break;
                    }
                }
            }
        }

        void PipelineImplDX12::writeResources(CommandListImplDX12& commandList, PipelineCache& cache, WriteFlags flags)
        {
            if (flags == WriteFlags::WriteDescriptors)
            {
                cache.constantUploads.clear();

                D3D12_CPU_DESCRIPTOR_HANDLE resourceCpuDescriptor = cache.descriptorTables->resourceHandles().cpuHandle();
                D3D12_GPU_DESCRIPTOR_HANDLE resourceGpuDescriptor = cache.descriptorTables->resourceHandles().gpuHandle();
                D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuDescriptor = cache.descriptorTables->samplerHandles().cpuHandle();
                D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuDescriptor = cache.descriptorTables->samplerHandles().gpuHandle();

                auto resourceHandleSize = cache.descriptorTables->resourceHandles().handleSize();
                auto samplerHandleSize = cache.descriptorTables->samplerHandles().handleSize();
                auto device = static_cast<DeviceImplDX12*>(m_device.native())->device();

                auto createNullResourceSRV = [&](engine::ResourceDimension dimension, engine::Format format)->D3D12_CPU_DESCRIPTOR_HANDLE
                {
                    if (m_device.nullResouces().nullTextureSRV.find(dimension) != m_device.nullResouces().nullTextureSRV.end() &&
                        m_device.nullResouces().nullTextureSRV[dimension].find(format) != m_device.nullResouces().nullTextureSRV[dimension].end())
                    {
                        return static_cast<TextureSRVImplDX12*>(m_device.nullResouces().nullTextureSRV[dimension][format].resource().m_impl)->native();
                    }
                    else
                    {
                        if (m_device.nullResouces().nullTextureSRV.find(dimension) == m_device.nullResouces().nullTextureSRV.end())
                            m_device.nullResouces().nullTextureSRV[dimension] = engine::unordered_map<engine::Format, TextureSRVOwner>();

                        m_device.nullResouces().nullTextureSRV[dimension][format] = m_device.createTextureSRV(TextureDescription()
                            .width(1)
                            .height(1)
                            .format(format)
                            .usage(ResourceUsage::GpuRead)
                            .dimension(dimension)
                            .name("Null TextureSRV"));

                        auto dimensionToStr = [](ResourceDimension dimension)->const char*
                        {
                            switch (dimension)
                            {
                            case ResourceDimension::Unknown: return "Unknown";
                            case ResourceDimension::Texture1D: return "Texture1D";
                            case ResourceDimension::Texture2D: return "Texture2D";
                            case ResourceDimension::Texture3D: return "Texture3D";
                            case ResourceDimension::Texture1DArray: return "Texture1DArray";
                            case ResourceDimension::Texture2DArray: return "Texture2DArray";
                            case ResourceDimension::TextureCube: return "TextureCube";
                            case ResourceDimension::TextureCubeArray: return "TextureCubeArray";
                            }
                            return "";
                        };

                        //LOG("Null format created. Format: %s, dimension: %s",
                        //    formatToString(format).c_str(),
                        //    dimensionToStr(dimension));

                        return static_cast<TextureSRVImplDX12*>(m_device.nullResouces().nullTextureSRV[dimension][format].resource().m_impl)->native();
                    }
                };

                auto createNullResourceUAV = [&](engine::ResourceDimension dimension, engine::Format format)->D3D12_CPU_DESCRIPTOR_HANDLE
                {
                    if (m_device.nullResouces().nullTextureUAV.find(dimension) != m_device.nullResouces().nullTextureUAV.end() &&
                        m_device.nullResouces().nullTextureUAV[dimension].find(format) != m_device.nullResouces().nullTextureUAV[dimension].end())
                    {
                        return static_cast<TextureUAVImplDX12*>(m_device.nullResouces().nullTextureUAV[dimension][format].resource().m_impl)->native();
                    }
                    else
                    {
                        if (m_device.nullResouces().nullTextureUAV.find(dimension) == m_device.nullResouces().nullTextureUAV.end())
                            m_device.nullResouces().nullTextureUAV[dimension] = engine::unordered_map<engine::Format, TextureUAVOwner>();

                        m_device.nullResouces().nullTextureUAV[dimension][format] = m_device.createTextureUAV(TextureDescription()
                            .width(1)
                            .height(1)
                            .format(format)
                            .usage(ResourceUsage::GpuReadWrite)
                            .dimension(dimension)
                            .name("Null TextureUAV"));

                        auto dimensionToStr = [](ResourceDimension dimension)->const char*
                        {
                            switch (dimension)
                            {
                            case ResourceDimension::Unknown: return "Unknown";
                            case ResourceDimension::Texture1D: return "Texture1D";
                            case ResourceDimension::Texture2D: return "Texture2D";
                            case ResourceDimension::Texture3D: return "Texture3D";
                            case ResourceDimension::Texture1DArray: return "Texture1DArray";
                            case ResourceDimension::Texture2DArray: return "Texture2DArray";
                            case ResourceDimension::TextureCube: return "TextureCube";
                            case ResourceDimension::TextureCubeArray: return "TextureCubeArray";
                            }
                            return "";
                        };

                        LOG("Null format created. Format: %s, dimension: %s",
                            formatToString(format).c_str(),
                            dimensionToStr(dimension));

                        return static_cast<TextureUAVImplDX12*>(m_device.nullResouces().nullTextureUAV[dimension][format].resource().m_impl)->native();
                    }
                };

                cache.tableBindings.clear();

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                D3D12_GPU_DESCRIPTOR_HANDLE origResourceCpu = resourceGpuDescriptor;
                D3D12_GPU_DESCRIPTOR_HANDLE origSamplerCpu = samplerGpuDescriptor;
#endif

                auto rootConstantCount = m_rootSignature->rootConstantCount();
                for (auto&& shader : m_shaders)
                {
                    const engine::vector<Sampler>& samplers = shader.shader->samplers();
                    if (samplers.size() > 0)
                        cache.tableBindings.emplace_back(TableBinding{ samplerGpuDescriptor, static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });
                    for (auto&& sampler : samplers)
                    {
                        if (sampler.valid())
                        {
                            device->CopyDescriptorsSimple(
                                1,
                                samplerCpuDescriptor,
                                static_cast<const SamplerImplDX12*>(sampler.native())->native(),
                                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                            LOG("SAMPLER. pool index: %i, descriptor table[%i]. writing sampler: %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                                static_cast<int>(static_cast<size_t>(samplerCpuDescriptor.ptr - DeviceImplGet::impl(m_device).heaps().shaderVisible_sampler->getCpuHeapStart().ptr) / samplerHandleSize),
                                static_cast<int>(cache.tableBindings.size() - 1),
                                SamplerImplGet::impl(sampler)->native().ptr,
                                static_cast<int>((samplerGpuDescriptor.ptr - cache.tableBindings.back().handle.ptr) / samplerHandleSize),
                                samplerCpuDescriptor.ptr,
                                samplerGpuDescriptor.ptr);
#endif

                            samplerCpuDescriptor.ptr += samplerHandleSize;
                            samplerGpuDescriptor.ptr += samplerHandleSize;
                        }
                        else
                        {
                            auto dev = static_cast<DeviceImplDX12*>(m_device.native());
                            device->CopyDescriptorsSimple(
                                1,
                                samplerCpuDescriptor,
                                static_cast<SamplerImplDX12*>(dev->nullResources().sampler.native())->native(),
                                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                            LOG("SAMPLER. pool index: %i, descriptor table[%i]. writing Null sampler: %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                                static_cast<int>(static_cast<size_t>(samplerCpuDescriptor.ptr - DeviceImplGet::impl(m_device).heaps().shaderVisible_sampler->getCpuHeapStart().ptr) / samplerHandleSize),
                                static_cast<int>(cache.tableBindings.size() - 1),
                                SamplerImplGet::impl(DeviceImplGet::impl(m_device).nullResources().sampler)->native().ptr,
                                static_cast<int>((samplerGpuDescriptor.ptr - cache.tableBindings.back().handle.ptr) / samplerHandleSize),
                                samplerCpuDescriptor.ptr,
                                samplerGpuDescriptor.ptr);
#endif


                            samplerCpuDescriptor.ptr += samplerHandleSize;
                            samplerGpuDescriptor.ptr += samplerHandleSize;

                        }
                    }
                    auto nonBindlessSrvBindings = false;
                    auto resGpuDescBeforeBindings = TableBinding{ resourceGpuDescriptor, static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) };
                    
                    for (auto&& srv : shader.shader->srvBindings())
                    {
                        if (srv.type == shaders::BindingType::SRVTexture)
                        {
                            nonBindlessSrvBindings = true;
                            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
                            auto& res = shader.shader->texture_srvs()[srv.index];
                            if (res.valid())
                                srvHandle = static_cast<TextureSRVImplDX12*>(res.m_impl)->native();
                            else
                                srvHandle = createNullResourceSRV(srv.dimension, srv.format);
                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                srvHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    for (auto&& srv : shader.shader->srvBindings())
                    {
                        if (srv.type == shaders::BindingType::SRVBuffer && shader.shader->buffer_srvs_is_structured()[srv.index])
                        {
                            nonBindlessSrvBindings = true;
                            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

                            auto& res = shader.shader->buffer_srvs()[srv.index];
                            if (res.valid())
                                srvHandle = static_cast<BufferSRVImplDX12*>(res.m_impl)->native();
                            else
                                srvHandle = static_cast<BufferSRVImplDX12*>(static_cast<DeviceImplDX12*>(m_device.native())->nullResources().bufferSRV.resource().m_impl)->native();

                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                srvHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    for (auto&& srv : shader.shader->srvBindings())
                    {
                        if (srv.type == shaders::BindingType::SRVBuffer && !shader.shader->buffer_srvs_is_structured()[srv.index])
                        {
                            nonBindlessSrvBindings = true;
                            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

                            auto& res = shader.shader->buffer_srvs()[srv.index];
                            if (res.valid())
                                srvHandle = static_cast<BufferSRVImplDX12*>(res.m_impl)->native();
                            else
                                srvHandle = static_cast<BufferSRVImplDX12*>(static_cast<DeviceImplDX12*>(m_device.native())->nullResources().bufferSRV.resource().m_impl)->native();

                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                srvHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    if (nonBindlessSrvBindings)
                        cache.tableBindings.emplace_back(resGpuDescBeforeBindings);

                    nonBindlessSrvBindings = false;
                    resGpuDescBeforeBindings = TableBinding{ resourceGpuDescriptor, static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) };

                    for (auto&& uav : shader.shader->uavBindings())
                    {
                        if (uav.type == shaders::BindingType::UAVTexture)
                        {
                            D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
                            nonBindlessSrvBindings = true;
                            auto& res = shader.shader->texture_uavs()[uav.index];
                            if (res.valid())
                                uavHandle = static_cast<TextureUAVImplDX12*>(res.m_impl)->native();
                            else
                                uavHandle = createNullResourceUAV(uav.dimension, uav.format);

                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                uavHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    for (auto&& uav : shader.shader->uavBindings())
                    {
                        if (uav.type == shaders::BindingType::UAVBuffer && shader.shader->buffer_uavs_is_structured()[uav.index])
                        {
                            D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
                            nonBindlessSrvBindings = true;
                            auto& res = shader.shader->buffer_uavs()[uav.index];
                            if (res.valid())
                                uavHandle = static_cast<BufferUAVImplDX12*>(res.m_impl)->cpuHandle();
                            else
                                uavHandle = static_cast<BufferUAVImplDX12*>(static_cast<DeviceImplDX12*>(m_device.native())->nullResources().bufferUAV.resource().m_impl)->cpuHandle();

                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                uavHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    for (auto&& uav : shader.shader->uavBindings())
                    {
                        if (uav.type == shaders::BindingType::UAVBuffer && !shader.shader->buffer_uavs_is_structured()[uav.index])
                        {
                            D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
                            nonBindlessSrvBindings = true;
                            auto& res = shader.shader->buffer_uavs()[uav.index];
                            if (res.valid())
                                uavHandle = static_cast<BufferUAVImplDX12*>(res.m_impl)->cpuHandle();
                            else
                                uavHandle = static_cast<BufferUAVImplDX12*>(static_cast<DeviceImplDX12*>(m_device.native())->nullResources().bufferUAV.resource().m_impl)->cpuHandle();

                            device->CopyDescriptorsSimple(
                                1,
                                resourceCpuDescriptor,
                                uavHandle,
                                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                            resourceCpuDescriptor.ptr += resourceHandleSize;
                            resourceGpuDescriptor.ptr += resourceHandleSize;
                        }
                    }

                    if (nonBindlessSrvBindings)
                        cache.tableBindings.emplace_back(resGpuDescBeforeBindings);

                    for (auto&& srv : shader.shader->srvBindings())
                    {
                        if (srv.type == shaders::BindingType::BindlessSRVTexture)
                        {
                            shader.shader->bindless_texture_srvs()[srv.index]->m_impl->updateDescriptors(m_device.native());
                            cache.tableBindings.emplace_back(TableBinding{
                                static_cast<BindlessTextureSRVImplDX12*>(shader.shader->bindless_texture_srvs()[srv.index]->m_impl)->descriptorTableGPUHandle(),
                                static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });
                        }
                    }

                    for (auto&& srv : shader.shader->srvBindings())
                    {
                        if (srv.type == shaders::BindingType::BindlessSRVBuffer)
                        {
                            shader.shader->bindless_buffer_srvs()[srv.index]->m_impl->updateDescriptors(m_device.native());
                            cache.tableBindings.emplace_back(TableBinding{
                                static_cast<BindlessBufferSRVImplDX12*>(shader.shader->bindless_buffer_srvs()[srv.index]->m_impl)->descriptorTableGPUHandle(),
                                static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });
                        }
                    }

                    for (auto&& uav : shader.shader->uavBindings())
                    {
                        if (uav.type == shaders::BindingType::BindlessUAVTexture)
                        {
                            shader.shader->bindless_texture_uavs()[uav.index]->m_impl->updateDescriptors(m_device.native());
                            cache.tableBindings.emplace_back(TableBinding{
                                static_cast<BindlessTextureUAVImplDX12*>(shader.shader->bindless_texture_uavs()[uav.index]->m_impl)->descriptorTableGPUHandle(),
                                static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });
                        }
                    }

                    for (auto&& uav : shader.shader->uavBindings())
                    {
                        if (uav.type == shaders::BindingType::BindlessUAVBuffer)
                        {
                            shader.shader->bindless_buffer_uavs()[uav.index]->m_impl->updateDescriptors(m_device.native());
                            cache.tableBindings.emplace_back(TableBinding{
                                static_cast<BindlessBufferUAVImplDX12*>(shader.shader->bindless_buffer_uavs()[uav.index]->m_impl)->descriptorTableGPUHandle(),
                                static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });
                        }
                    }

                    auto& constants = const_cast<shaders::Shader*>(shader.shader)->constants();

                    if (constants.size() > 0)
                        cache.tableBindings.emplace_back(TableBinding{ resourceGpuDescriptor, static_cast<UINT>(cache.tableBindings.size() + rootConstantCount) });

                    engine::vector<engine::unique_ptr<BufferCBVOwner>>& cacheConstants = cache.constants[shader.visibility];

                    if (cacheConstants.size() != constants.size())
                        cacheConstants.resize(constants.size());

                    for (int i = 0; i < constants.size(); ++i)
                    {
                        auto& constant = constants[i];
                        if (!cacheConstants[i])
                        {
                            auto initial = BufferDescription::InitialData(
                                constant.range,
                                D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

                            auto cbv = m_device.createBufferCBV(BufferDescription()
                                .name(constant.name)
                                .usage(ResourceUsage::GpuReadWrite)
                                .elementSize(initial.elementSize)
                                .elements(initial.elements));

                            cacheConstants[i] = engine::make_unique<BufferCBVOwner>(cbv);
                        }

                        auto rangeSize = static_cast<int>(constant.range.sizeBytes());
                        auto bufferSize = static_cast<int>(cacheConstants[i]->resource().desc().elements * cacheConstants[i]->resource().desc().elementSize);

                        ASSERT(bufferSize >= rangeSize, "Buffer too small");

                        cache.constantUploads.emplace_back(ConstantUpdate{ *cacheConstants[i] , constant.range });

                        static_cast<DeviceImplDX12*>(m_device.native())->uploadBuffer(commandList, *cacheConstants[i], constant.range, 0);
                        commandList.transition(*cacheConstants[i], ResourceState::VertexAndConstantBuffer);

                        static_cast<DeviceImplDX12*>(m_device.native())->device()->CopyDescriptorsSimple(
                            1,
                            resourceCpuDescriptor,
                            static_cast<BufferCBVImplDX12*>(cacheConstants[i]->resource().m_impl)->native(),
                            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                        resourceCpuDescriptor.ptr += resourceHandleSize;
                        resourceGpuDescriptor.ptr += resourceHandleSize;
                    }
                }
            }
            else
            {
                for (auto&& constUpdate : cache.constantUploads)
                {
                    static_cast<DeviceImplDX12*>(m_device.native())->uploadBuffer(commandList, constUpdate.buffer, constUpdate.range, 0);
                    commandList.transition(constUpdate.buffer, ResourceState::VertexAndConstantBuffer);
                }
            }
        }
    }
}
