#include "GlobalTestFixture.h"

#include "platform/window/Window.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Semaphore.h"
#include "containers/memory.h"

using namespace engine;
using namespace platform;
using namespace engine;

class TestSemaphore : public ::testing::Test
{
};

TEST(TestSemaphore, CreateSemaphore)
{
    GlobalEnvironment& env = *envPtr;
    Semaphore semaphore = env.device().createSemaphore();
    Semaphore copy = std::move(semaphore);
}
