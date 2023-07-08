#include "engine/graphics/metal/MetalPipeline.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalShaderBinary.h"
#include "engine/graphics/metal/MetalConversions.h"
#include "engine/graphics/metal/MetalRootSignature.h"
#include "engine/graphics/metal/MetalDevice.h"

#include "engine/graphics/ShaderBinary.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        PipelineImpl::PipelineImpl(const Device& device)
        : m_device{ device }
        , m_rootSignature{ nullptr }
        , m_pipelineState { nullptr }
        //, m_pipelineStateDesc{ new D3D12_GRAPHICS_PIPELINE_STATE_DESC() }
        {
        }
        
        PipelineImpl::~PipelineImpl()
        {
            if (m_pipelineStateDesc)
            {
                //delete m_pipelineStateDesc;
                m_pipelineStateDesc = nullptr;
            }
            if (m_pipelineState)
            {
                //m_pipelineState->Release();
                m_pipelineState = nullptr;
            }
            if (m_rootSignature)
            {
                //m_rootSignature->Release();
                m_rootSignature = nullptr;
            }
        }
        
        void* PipelineImpl::native() const
        {
            return m_pipelineState;
        }
        
        void PipelineImpl::setRootSignature(const RootSignature& signature)
        {
            //m_rootSignature->AddRef();
        }
        
        void PipelineImpl::setBlendState(const BlendDescription& desc)
        {
            //m_pipelineStateDesc->BlendState = dxBlendDesc(desc);
        }
        
        void PipelineImpl::setRasterizerState(const RasterizerDescription& desc)
        {
            //m_pipelineStateDesc->RasterizerState = dxRasterizerDesc(desc);
        }
        
        void PipelineImpl::setDepthStencilState(const DepthStencilDescription& desc)
        {
            //m_pipelineStateDesc->DepthStencilState = dxDepthStencilDesc(desc);
        }
        
        void PipelineImpl::setSampleMask(unsigned int mask)
        {
            //m_pipelineStateDesc->SampleMask = mask;
        }
        
        void PipelineImpl::setPrimitiveTopologyType(PrimitiveTopologyType type)
        {
            // Can't draw with undefined topology
            //ASSERT(dxPrimitiveTopologyType(type) != D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED);
            //m_pipelineStateDesc->PrimitiveTopologyType = dxPrimitiveTopologyType(type);
        }
        
        void PipelineImpl::setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            setRenderTargetFormats(1, &RTVFormat, DSVFormat, msaaCount, msaaQuality);
        }
        
        void PipelineImpl::setRenderTargetFormats(unsigned int numRTVs, const Format* RTVFormats, Format DSVFormat, unsigned int msaaCount, unsigned int msaaQuality)
        {
            // Null format array conflicts with non-zero length
            ASSERT(numRTVs == 0 || RTVFormats != nullptr);
            /*for (UINT i = 0; i < numRTVs; ++i)
                m_pipelineStateDesc->RTVFormats[i] = dxFormat(RTVFormats[i]);
            for (UINT i = numRTVs; i < m_pipelineStateDesc->NumRenderTargets; ++i)
                m_pipelineStateDesc->RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
            m_pipelineStateDesc->NumRenderTargets = numRTVs;
            m_pipelineStateDesc->DSVFormat = dxFormat(DSVFormat);
            m_pipelineStateDesc->SampleDesc.Count = msaaCount;
            m_pipelineStateDesc->SampleDesc.Quality = msaaQuality;*/
        }
        
        void PipelineImpl::setInputLayout(unsigned int numElements, const InputElementDescription* inputElementDescs)
        {
            /*m_pipelineStateDesc->InputLayout.NumElements = numElements;
            if (numElements > 0)
            {
                D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * numElements);
                memcpy(NewElements, inputElementDescs, numElements * sizeof(D3D12_INPUT_ELEMENT_DESC));
                m_inputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
            }
            else
            {
                m_inputLayouts = nullptr;
            }*/
        }
        
        void PipelineImpl::setPrimitiveRestart(IndexBufferStripCutValue value)
        {
            //m_pipelineStateDesc->IBStripCutValue = dxIndexBufferStripCutValue(value);
        }
        
        void PipelineImpl::setVertexShader(const ShaderBinary& shaderBinary)
        {
            //m_pipelineStateDesc->VS = ShaderBinaryImplGet::impl(shaderBinary).native();
        }
        
        void PipelineImpl::setPixelShader(const ShaderBinary& shaderBinary)
        {
            //m_pipelineStateDesc->PS = ShaderBinaryImplGet::impl(shaderBinary).native();
        }
        
        void PipelineImpl::setGeometryShader(const ShaderBinary& shaderBinary)
        {
            //m_pipelineStateDesc->GS = ShaderBinaryImplGet::impl(shaderBinary).native();
        }
        
        void PipelineImpl::setHullShader(const ShaderBinary& shaderBinary)
        {
            //m_pipelineStateDesc->HS = ShaderBinaryImplGet::impl(shaderBinary).native();
        }
        
        void PipelineImpl::setDomainShader(const ShaderBinary& shaderBinary)
        {
            //m_pipelineStateDesc->DS = ShaderBinaryImplGet::impl(shaderBinary).native();
        }
        
        void PipelineImpl::finalize()
        {
            // Make sure the root signature is finalized first
            //m_pipelineStateDesc->pRootSignature = m_rootSignature;
            //ASSERT(m_pipelineStateDesc->pRootSignature != nullptr);
            
            //m_pipelineStateDesc->InputLayout.pInputElementDescs = nullptr;
            //size_t HashCode = Utility::HashState(&m_pipelineStateDesc);
            //HashCode = Utility::HashState(m_InputLayouts.get(), m_pipelineStateDesc->InputLayout.NumElements, HashCode);
            //m_pipelineStateDesc->InputLayout.pInputElementDescs = m_inputLayouts.get();
            
            /*ID3D12PipelineState** PSORef = nullptr;
             bool firstCompile = false;
             {
             static mutex s_HashMapMutex;
             lock_guard<mutex> CS(s_HashMapMutex);
             auto iter = s_GraphicsPSOHashMap.find(HashCode);
             
             // Reserve space so the next inquiry will find that someone got here first.
             if (iter == s_GraphicsPSOHashMap.end())
             {
             firstCompile = true;
             PSORef = s_GraphicsPSOHashMap[HashCode].GetAddressOf();
             }
             else
             PSORef = iter->second.GetAddressOf();
             }
             
             if (firstCompile)
             {*/
     /*       ASSERT(SUCCEEDED(
                             DeviceImplGet::impl(m_device).device()->CreateGraphicsPipelineState(
                                                                                                 m_pipelineStateDesc,
                                                                                                 DARKNESS_IID_PPV_ARGS(&m_pipelineState))));*/
            //s_GraphicsPSOHashMap[HashCode].Attach(m_pipelineState));
            /*}
             else
             {
             while (*PSORef == nullptr)
             this_thread::yield();
             m_PSO = *PSORef;
             }*/
        }
    }
}
