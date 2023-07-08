#include "GlobalTestFixture.h"

using namespace engine;
using namespace platform;
using namespace engine;

TEST(TestTexture, TestClearRenderTargetTexture)
{
    GlobalEnvironment& env = *envPtr;

    // start drawing
    auto cmdBuffer = env.device().createCommandList("TestClearRenderTargetTexture");
    cmdBuffer.begin();
    cmdBuffer.clearRenderTargetView(
        env.currentRTV(),
        Color4f(0.0f, 1.0f, 0.0f, 1.0f));
    cmdBuffer.end();

    env.submit(cmdBuffer);
    env.present();
    env.device().waitForIdle();
}
