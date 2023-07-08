#include "GlobalTestFixture.h"
#include "gtest/gtest.h"
#include "tools/Octree.h"
#include "engine/rendering/ImguiRenderer.h"
#include <random>

using namespace engine;
using namespace engine;

TEST(TestOctree, PreserveRoot)
{
    Octree<int> octree(engine::BoundingBox{
        Vector3f(-10.0f, -10.0f, -10.0f),
        Vector3f(10.0f, 10.0f, 10.0f) });

    octree.insert(Vector3f{ 1.0f, 1.0f, 1.0f }, 1);
    octree.erase(Vector3f{ 1.0f, 1.0f, 1.0f }, 1);
    EXPECT_EQ(octree.type(), OctreeNodeType::Root);
}

TEST(TestOctree, ClosestPayload)
{
    int index = 0;
    GlobalEnvironment& env = *envPtr;

    Scene scene;
    {
        auto cameraNode = engine::make_shared<engine::SceneNode>();
        auto cameraTransform = engine::make_shared<engine::Transform>();
        cameraNode->name("Camera");
        cameraNode->addComponent(cameraTransform);
        cameraNode->addComponent(engine::make_shared<engine::Camera>());
        cameraNode->addComponent(engine::make_shared<engine::PostprocessComponent>());
        cameraNode->getComponent<Camera>()->pbrShadingModel(true);
        cameraNode->getComponent<Camera>()->exposure(1.0f);
        scene.root()->addChild(cameraNode);
        cameraTransform->position({ 0.0f, 20.0f, 40.0f });
        cameraTransform->rotation(Quaternionf::fromEulerAngles({ -30.0f, 0.0f, 0.0f }));

        env.inputManager().setCamera(cameraNode->getComponent<Camera>().get());
    }

    auto& flatScene = scene.flatten(false, 0.0f);

    ImGuiContext* context = ImGui::CreateContext();
    engine::ImguiRenderer imguiRenderer(env.device(), context);

    TextureDSV dsv;

    do
    {
        Octree<int> octree(engine::BoundingBox{
            Vector3f(-10.0f, -10.0f, -10.0f),
            Vector3f(10.0f, 10.0f, 10.0f) });

        LOG("index: %i", index);
        std::random_device r;
        std::default_random_engine rnd(index);
        std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

        vector<Vector3f> points;
        for (int i = 0; i < 1000; ++i)
        {
            points.emplace_back(Vector3f{ dist(rnd), dist(rnd), dist(rnd) });
        }

        for (int i = 0; i < points.size(); ++i)
        {
            octree.insert(points[i], i);
        }

        vector<vector<float>> bruteDistances;
        for (int a = 0; a < points.size(); ++a)
        {
            vector<float> distances;
            for (int b = 0; b < points.size(); ++b)
            {
                distances.emplace_back((points[a] - points[b]).magnitude());
            }
            bruteDistances.emplace_back(std::move(distances));
        }

        auto getBruteClosest = [&](int a)->int
        {
            float closest = std::numeric_limits<float>::max();
            int closestIndex = -1;
            for (int i = 0; i < bruteDistances[a].size(); ++i)
            {
                if ((a != i) && (bruteDistances[a][i] < closest))
                {
                    closest = bruteDistances[a][i];
                    closestIndex = i;
                }
            }
            return closestIndex;
        };


            auto cmd = env.device().createCommandList("ClosestPayload");

            cmd.clearRenderTargetView(env.currentRTV());

            imguiRenderer.startFrame(env.device(), 1.0f / 60.0f);

            int cc = -1;
            int ca = -1;
            int cb = -1;

#ifdef DEBUG_OCTREE
            usedBoundingBoxes.clear();
            searchBoundingBoxes.clear();
            initialSearchBoundingBoxes.clear();
#endif

            for (int i = 0; i < points.size(); ++i)
            {
                auto a = getBruteClosest(i);
                auto b = octree.getClosestPayload(points[i]);
#ifdef DEBUG_OCTREE
                ca = a;
                cb = b;
#endif
                EXPECT_EQ(a, b);
#ifdef DEBUG_OCTREE
                if (a != b)
                {
                    usedBoundingBoxes.clear();
                    searchBoundingBoxes.clear();
                    initialSearchBoundingBoxes.clear();
                    cc = i;
                    ca = getBruteClosest(i);
                    cb = octree.getClosestPayload(points[i]);
                    break;
                }
#endif
            }
            ++index;

            renderBoxes(flatScene, octree.recursiveBoundingBoxes(), 0x881080FF);
            engine::vector<BoundingSphere> other_spheres;
            for (int i = 0; i < points.size(); ++i)
            {
                if(i != cc && i != ca && i != cb)
                    other_spheres.emplace_back(BoundingSphere(points[i], 0.1f));
            }
            // A B G R
            imguiRenderer.renderSpheres(flatScene, other_spheres, 0xFF555555);

#ifdef DEBUG_OCTREE
            if (cc != -1 && ca != -1 && cb != -1)
            {
                imguiRenderer.renderSpheres(flatScene, { BoundingSphere{ points[cc], 0.1f } }, 0xFFFFFFFF);
                imguiRenderer.renderSpheres(flatScene, { BoundingSphere{ points[ca], 0.1f } }, 0xFF00FF00);
                imguiRenderer.renderSpheres(flatScene, { BoundingSphere{ points[cb], 0.1f } }, 0xFF0000FF);

                imguiRenderer.renderBoxes(flatScene, searchBoundingBoxes, 0xFFFF0000);
                imguiRenderer.renderBoxes(flatScene, usedBoundingBoxes, 0x880400FF);
                imguiRenderer.renderBoxes(flatScene, initialSearchBoundingBoxes, 0xFFFFFF00);
            }
#endif

            imguiRenderer.render(flatScene);
            imguiRenderer.endFrame(env.device(), env.currentRTV(), dsv, cmd);

            //auto flatScene.cameras[flatScene.selectedCamera]->lookAt

            env.submit(cmd);
            env.present();
    } while (env.canContinue(false));

    ImGui::DestroyContext(context);
}
