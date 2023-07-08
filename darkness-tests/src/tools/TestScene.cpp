#include "gtest/gtest.h"
#include "engine/SceneV2.h"

using namespace engine;
using namespace engine::experimental;

TEST(TestScene, SceneCreate)
{
    Scene scene;
    scene.test();
}