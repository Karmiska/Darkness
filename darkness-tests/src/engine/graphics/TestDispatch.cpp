#include "GlobalTestFixture.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"

#include "shaders/core/tests/HLSLTest.h"
#include "shaders/core/tests/DrawWithoutBuffers.h"
#include "shaders/core/tests/DrawWithConstants.h"
#include "shaders/core/tests/DrawWithPixelShaderResources.h"
#include "shaders/core/tests/DrawWithTexture.h"
#include "shaders/core/tests/DispatchBasic.h"

#include <iostream>
#include <fstream>

TEST(TestDraw, DispatchBasic)
{
    GlobalEnvironment& env = *envPtr;

    auto inputElementCount = 150;
    auto outputElementCount = 250;

	engine::vector<uint32_t> signal;
	for (int i = 0; i < inputElementCount; ++i) { signal.emplace_back(i); }

	auto output = env.device().createBufferUAV(BufferDescription().elements(250).format(Format::R32_UINT).name("Dispatch output"));
	auto input = env.device().createBufferSRV(BufferDescription()
		.format(Format::R32_UINT)
		.name("Dispatch input")
		.setInitialData(BufferDescription::InitialData(signal)));

    auto basicDispatch = env.device().createPipeline<DispatchBasic>();
	basicDispatch.cs.outputs = output;
    basicDispatch.cs.size.x = inputElementCount;
	basicDispatch.cs.inputs = input;

    do
    {
        auto cmdBuffer = env.device().createCommandList("DispatchBasic");

        cmdBuffer.clearBuffer(basicDispatch.cs.outputs, 0, 0, outputElementCount);

        cmdBuffer.bindPipe(basicDispatch);
        cmdBuffer.dispatch(roundUpToMultiple(static_cast<int>(inputElementCount), static_cast<size_t>(64)) / 64, 1, 1);

        env.submit(cmdBuffer);
        env.present();
    } while (env.canContinue(true));
    env.device().waitForIdle();
}
