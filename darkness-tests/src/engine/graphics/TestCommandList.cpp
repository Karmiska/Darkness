#include "GlobalTestFixture.h"
#include "shaders/core/tests/Mrt.h"
#include "shaders/core/tests/MrtVisualize.h"

TEST(TestCommandList, SetRenderTargets_multiple)
{
    GlobalEnvironment& env = *envPtr;
    Device& dev = env.device();
    
    uint32_t rtvSize = 1024;
    uint32_t mrtCount = 4;
    engine::vector<TextureRTVOwner> targets;
	engine::vector<TextureRTV> targetViews;
    engine::vector<TextureSRVOwner> targetSRVs;
    engine::vector<engine::Format> rtvFormats;
    for (uint32_t i = 0; i < mrtCount; ++i)
    {
        rtvFormats.emplace_back(Format::R8G8B8A8_UNORM);
        targets.emplace_back(dev.createTextureRTV(TextureDescription()
            .width(rtvSize)
            .height(rtvSize)
            .format(Format::R8G8B8A8_UNORM)
            .usage(ResourceUsage::GpuRenderTargetReadWrite)
            .name("RenderTargetTest RTV")));
		targetViews.emplace_back(targets.back().resource());
    }

    for (uint32_t i = 0; i < mrtCount; ++i)
        targetSRVs.emplace_back(dev.createTextureSRV(targets[i]));

    auto pipe = dev.createPipeline<Mrt>();
    pipe.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleStrip);
    pipe.setRasterizerState(RasterizerDescription().frontCounterClockwise(false));
    

    auto visualizePipe = dev.createPipeline<MrtVisualize>();
    visualizePipe.setPrimitiveTopologyType(PrimitiveTopologyType::TriangleList);
    visualizePipe.vs.width = static_cast<float>(rtvSize);
    visualizePipe.vs.height = static_cast<float>(rtvSize);
	visualizePipe.ps.tex_sampler = dev.createSampler(SamplerDescription());

    visualizePipe.ps.mrt = dev.createBindlessTextureSRV();
    for (uint32_t i = 0; i < mrtCount; ++i)
        visualizePipe.ps.mrt.push(targetSRVs[i]);

    do
    {
        {
            auto cmd = dev.createCommandList("TestCommandList");
            for (uint32_t i = 0; i < mrtCount; ++i)
                cmd.clearRenderTargetView(targets[i]);
            cmd.setRenderTargets(targetViews);
            cmd.bindPipe(pipe);

            cmd.draw(4);
            env.submit(cmd);
        }

        {
            auto cmd = dev.createCommandList("TestCommandList2");
            cmd.clearRenderTargetView(env.currentRTV(), {0.0f, 0.1f, 0.1f, 0.1f});

            cmd.setRenderTargets({ env.currentRTV() });
            cmd.bindPipe(visualizePipe);

            cmd.draw(24);
            env.submit(cmd);
        }

        env.present();
        env.window().processMessages();
    } while (env.canContinue(false));

    env.device().waitForIdle();

}

