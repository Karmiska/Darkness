#include "GlobalTestFixture.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "platform/Environment.h"
#include "tools/PathTools.h"

#include "shaders/core/tests/HLSLTest.h"
#include "shaders/core/tests/DrawWithoutBuffers.h"
#include "shaders/core/tests/DrawWithConstants.h"
#include "shaders/core/tests/DrawWithPixelShaderResources.h"
#include "shaders/core/tests/DrawWithTexture.h"

//#include "shaders/core/dxr/RayTraceTest.h"

#include <iostream>
#include <fstream>

TEST(TestRendering, DISABLED_DrawDxr)
{
    /*GlobalEnvironment& env = *envPtr;

    auto positionSRV = env.device().createBufferSRV(BufferDescription()
        .name("position")
        .format(Format::R32G32B32_FLOAT)
        .setInitialData(BufferDescription::InitialData(engine::vector<Float4>{
            { -0.5f, 0.5f, 0.0f }, { 0.0f, -0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f }  })));

    auto colorSRV = env.device().createBufferSRV(BufferDescription()
        .name("colorStaging")
        .format(Format::R32G32B32A32_FLOAT)
        .setInitialData(BufferDescription::InitialData(engine::vector<Float4>{
            { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } })));

    auto indexBufferView = env.device().createBufferIBV(BufferDescription()
        .name("indexStaging")
        .setInitialData(BufferDescription::InitialData(engine::vector<uint32_t>{0, 1, 2})));

    auto accelerationStructure = env.device().createRaytracingAccelerationStructure(positionSRV, indexBufferView, BufferDescription());

    auto output = env.device().createTextureUAV(TextureDescription()
        .width(env.swapChain().size().width)
        .height(env.swapChain().size().height)
        .format(Format::R8G8B8A8_UNORM)
        .dimension(ResourceDimension::Texture2D)
        .name("Dxr output")
    );

    // create pipeline
    auto rayTraceTest = env.device().createPipeline<shaders::RayTraceTest>();

    rayTraceTest.rg.SceneBVH = accelerationStructure;
    rayTraceTest.rg.gOutput = output;
    
    // presentation
    do
    {
        engine::CommandList cmdBuffer = env.device().createCommandList("TEST DrawIndexed");
        cmdBuffer.clearRenderTargetView(env.currentRTV(), { 0.0f, 0.0f, 0.0f, 1.0f });
        cmdBuffer.setRenderTargets({ env.currentRTV() });
        cmdBuffer.bindPipe(rayTraceTest);
        cmdBuffer.drawIndexedInstanced(indexBufferView, static_cast<uint32_t>(3), 1, 0, 0, 0);
        
        cmdBuffer.transition(env.currentRTV(), ResourceState::Present);

        env.submit(cmdBuffer);
        env.present();
    } while (env.canContinue(true));

    // close
    env.device().waitForIdle();
    */
}
