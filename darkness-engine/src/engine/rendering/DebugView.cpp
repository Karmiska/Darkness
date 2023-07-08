#include "engine/rendering/DebugView.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "tools/Measure.h"

using namespace tools;

namespace engine
{
    DebugView::DebugView(Device& device)
        : m_device{ device }
        , m_pipeline{ device.createPipeline<shaders::DebugViewer>() }
    {
        m_pipeline.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
        m_pipeline.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
        m_pipeline.ps.tex_sampler = device.createSampler(SamplerDescription().filter(engine::Filter::Bilinear));
    }

    void DebugView::render(
        CommandList& cmd,
		TextureRTV currentRenderTarget,
		TextureSRV debugSRV)
    {
        if (debugSRV.valid())
        {
            cmd.setRenderTargets({ currentRenderTarget });

            m_pipeline.ps.tex = debugSRV;
            m_pipeline.vs.width = static_cast<float>(currentRenderTarget.width());
            m_pipeline.vs.height = static_cast<float>(currentRenderTarget.height());

            cmd.bindPipe(m_pipeline);
            cmd.draw(4u);
        }
    }
}
