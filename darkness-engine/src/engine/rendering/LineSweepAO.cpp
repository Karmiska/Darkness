#include "engine/rendering/LineSweepAO.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "tools/Measure.h"

using namespace tools;

namespace engine
{
    LineSweepAO::LineSweepAO(Device& device)
        : m_device{ device }
        , m_pipeline{ device.createPipeline<shaders::LSAO>() }
    {
        
    }

    void LineSweepAO::render(
        Device& /*device*/,
        TextureRTV& /*currentRenderTarget*/,
        TextureSRV& depthBuffer,
        CommandList& /*cmd*/
    )
    {
        m_pipeline.cs.depth = depthBuffer;
    }
}
