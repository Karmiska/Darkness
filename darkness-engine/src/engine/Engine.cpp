#include "engine/Engine.h"
#include "engine/graphics/Common.h"
#include "engine/primitives/Matrix4.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/ProbeComponent.h"
#include "components/TerrainComponent.h"
//#include "plugins/PluginManager.h"
#include "engine/graphics/Barrier.h"
#include "imgui.h"
//#include "external/ImGuizmo/ImGuizmo.h"
#include "tools/measure.h"
#include "tools/RefCounted.h"
#include "containers/vector.h"
#include <chrono>
#include <algorithm>

#include "btBulletDynamicsCommon.h"
//#include "entityx.h"

using namespace engine;

int RefreshPipelines = 0;

thread_local ImGuiContext* MyImGuiTLS;

Engine::Engine(
    engine::shared_ptr<platform::Window> window,
    const engine::string& shaderRootPath,
    GraphicsApi api,
    EngineMode mode,
    const char* name,
    engine::shared_ptr<engine::Scene> scene,
    engine::shared_ptr<InputManager> inputManager)
    : m_mode{ mode }
    , m_window{ window }
    , m_renderSetup{ engine::make_unique<RenderSetup>(window, api, mode, name, true, "", [this](const engine::vector<engine::string>& messages)
        {
            m_logWindow.pushMessages(messages);
        }) }
    , m_shaderRootPath{ shaderRootPath }
    , m_scene{ scene != nullptr ? scene : engine::make_shared<engine::Scene>() }
    , m_cameraSizeRefresh{ true }
    , m_virtualResolution{ m_renderSetup->device().width(), m_renderSetup->device().height() }
    , m_viewportRenderer{ engine::make_unique<ViewportRenderer>(m_renderSetup->device(), m_virtualResolution.x, m_virtualResolution.y) }
    , m_imguiContext{ engine::shared_ptr<ImGuiContext>(ImGui::CreateContext(), [](ImGuiContext* context) { ImGui::DestroyContext(context); }) }
    , m_imguiRenderer{ engine::make_unique<ImguiRenderer>( m_renderSetup->device(), m_imguiContext.get() ) }
    , m_debugViewer{ engine::make_unique<DebugView>(m_renderSetup->device()) }
    , m_cycleTransforms{ engine::make_unique<engine::Pipeline<engine::shaders::CycleTransforms>>(m_renderSetup->device().createPipeline<shaders::CycleTransforms>()) }
    , m_frameCapturer{ (m_mode == EngineMode::OwnThreadNoPresent) ? engine::make_unique<engine::FrameCpuCapturer>(m_renderSetup->device()) : nullptr }
    , m_lightData{ engine::make_shared<LightData>() }
    , m_inputManager{ inputManager != nullptr ? inputManager : engine::make_shared<InputManager>(m_renderSetup->device().width(), m_renderSetup->device().height()) }
    //, m_cameraTransform{ engine::make_shared<engine::Transform>() }
    //, m_camera{ engine::make_unique<Camera>(m_cameraTransform) }
	, m_cameraInput{ engine::make_unique<CameraInput>() }
    , m_cameraInputActive{ false }
    , m_lastEnvironmentMap{ "" }
    , m_simulating{ false }
    , m_lastPickedObject{ -1 }
    , m_selectedObject{ -1 }
    , m_updateEnvironmentOnNextFrame{ -1 }
    , m_collisionShapes{ engine::shared_ptr<void>(static_cast<void*>(new btAlignedObjectArray<btCollisionShape*>()), [](void* ptr)
        {
            auto p = reinterpret_cast<btAlignedObjectArray<btCollisionShape*>*>(ptr);
            delete p;
        }) }
	, m_engineRunning{ true }
	, m_engineExited{ false }
	, m_requestEngineStallBeginEnd{ false }
	, m_engineStalled{ false }
    , m_engineThread{ ((m_mode == EngineMode::OwnThread) || (m_mode == EngineMode::OwnThreadNoPresent)) ? engine::make_unique<std::thread>([&]()
    {
        m_engineRunning = true;
        while (true)
        {
            if (stepEngine())
                break;
        }
		m_engineExited = true;
    }) : nullptr }
    //, m_lastFrame{}
	, m_sceneLoadRequest{ false }
	, m_sceneToLoad{ "" }
{
    /*entityx::EntityX ex;
    entityx::Entity entity = ex.entities.create();
    entity.assign<Vector3f>(0.0f, 0.0f, 0.0f);

    entity.destroy();*/

#if 1
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    m_collisionConfiguration = engine::make_shared<btDefaultCollisionConfiguration>();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = engine::make_shared<btCollisionDispatcher>(m_collisionConfiguration.get());

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    m_overlappingPairCache = engine::make_shared<btDbvtBroadphase>();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    m_solver = engine::make_shared<btSequentialImpulseConstraintSolver>();

    m_dynamicsWorld = engine::make_shared<btDiscreteDynamicsWorld>(m_dispatcher.get(), m_overlappingPairCache.get(), m_solver.get(), m_collisionConfiguration.get());

    m_dynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

    ///-----initialization_end-----

    //keep track of the shapes, we release memory at exit.
    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btAlignedObjectArray<btCollisionShape*>* collisionShapes = reinterpret_cast<btAlignedObjectArray<btCollisionShape*>*>(m_collisionShapes.get());
    
    /*ProcessResourcePackage package;
    m_resourceHost.processResources(package);*/

    ///create a few basic rigid bodies

    //the ground is a cube of side 100 at position y = -56.
    //the sphere will hit it at y = -6, with center at -5
    {
        btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

        collisionShapes->push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -50.0, 0));

        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            groundShape->calculateLocalInertia(mass, localInertia);

        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }
#endif

    //m_defaultMaterial->gpuRefresh(m_renderSetup->device());

    //m_camera->width(m_window->width());
    //m_camera->height(m_window->height());
    //m_camera->nearPlane(1.0f);
    //m_camera->farPlane(0.0f);
    //m_camera->fieldOfView(90.0f);
    //m_camera->projection(Projection::Perspective);
    //m_camera->rotation(Quaternionf::fromEulerAngles(0.0f, 0.0f, 0.0f));
    //m_camera->position({ 0.0f, 2.0f, -10.0f});
}

bool Engine::stepEngine()
{
    auto engineRenderedFrame = engineRun();
    if (!engineRenderedFrame)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
        std::lock_guard<std::mutex> lock(m_engineMutex);
        if (!m_engineRunning)
            return true;

        if (m_viewportRenderer->virtualResolutionChange(true))
        {
            m_virtualResolution = m_viewportRenderer->virtualResolution();
            refresh(false);
        }

        if (m_refreshSize)
        {
            m_refreshSize = false;
            refresh();
        }
        if (m_sceneLoadRequest)
        {
            m_sceneLoadRequest = false;
            m_scene->loadFrom(m_sceneToLoad);
        }
    }

    m_engineStalled = false;
    {
        std::lock_guard<std::mutex> lock(m_engineMutex);
        m_engineStalled = m_requestEngineStallBeginEnd;
        m_requestEngineStallBeginEnd = false;
    }
    if (m_engineStalled)
    {
        bool startAgain = false;
        while (!startAgain)
        {
            {
                std::lock_guard<std::mutex> lock(m_engineMutex);
                startAgain = m_requestEngineStallBeginEnd;
                m_requestEngineStallBeginEnd = false;

                if (startAgain)
                    m_engineStalled = false;
            }
        }

    }
    return false;
}

bool Engine::engineRun()
{
    return update();
}

Engine::~Engine()
{
    {
        std::lock_guard<std::mutex> lock(m_engineMutex);
        m_engineRunning = false;
    }

	bool engineExit = m_engineThread == nullptr ? true : false;
	while (!engineExit)
	{
		{
			std::lock_guard<std::mutex> lock(m_engineMutex);
			engineExit = m_engineExited;
		}
		m_renderSetup->window().processMessages();
	}
    if(m_engineThread != nullptr)
        m_engineThread->join();

    shutdown();
}

void Engine::pauseEngine(bool val)
{
    if (val)
    {
        m_engineMutex.lock();
    }
    else
    {
        m_engineMutex.unlock();
    }
}

void Engine::refreshSize()
{
    std::lock_guard<std::mutex> lock(m_engineMutex);
    m_refreshSize = true;
}

void Engine::loadScene(const engine::string& filePath)
{
	std::lock_guard<std::mutex> lock(m_engineMutex);
	m_sceneLoadRequest = true;
	m_sceneToLoad = filePath;
}

engine::shared_ptr<engine::Camera> Engine::camera()
{
    return m_scene->getComponent<engine::Camera>();
}

engine::Scene& Engine::scene()
{
    return *m_scene;
}

CpuTexture Engine::grabRTV()
{
    return m_frameCapturer->latestFrame();
}

bool Engine::update()
{
    auto result = render();
    if (m_mode == EngineMode::OwnThread && m_renderSetup->swapChain().needRefresh())
    {
        refresh();
    }
	return result;
}

void Engine::playClicked(bool value)
{
    m_simulating = value;
    m_scene->root()->invalidate();
}

float Engine::delta()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastUpdate).count()) / 1000000000.0;
    m_lastUpdate = now;

    if(m_simulating)
        m_dynamicsWorld->stepSimulation(static_cast<float>(duration), 10);

    //return 1.0f / 60.0f;
    return static_cast<float>(duration);
}

void Engine::resetCameraSize()
{
    auto flatScene = m_scene->flatten(false, 0.0f);
    for(auto&& camera : flatScene.cameras)
    {
        camera->width(m_renderSetup->window().width());
        camera->height(m_renderSetup->window().height());
    }
}

engine::shared_ptr<engine::SceneNode> Engine::grabSelected()
{
    engine::shared_ptr<engine::SceneNode> res = m_lastPickedNode;
    m_lastPickedNode = nullptr;
    return res;
}

void Engine::setSelected(engine::shared_ptr<engine::SceneNode> node)
{
    auto mesh = node->getComponent<MeshRendererComponent>();
    if (mesh)
    {
        auto id = mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex;
        m_viewportRenderer->setSelectedObject(id);
        m_selectedObject = id;
    }
    else
        m_selectedObject = node->id();
}

static int DebugMeasureLevel = 0;

void drawMeasureImgui(engine::vector<engine::QueryResultTicks>& results)
{
	++DebugMeasureLevel;
    for (auto& res : results)
    {
		//if(DebugMeasureLevel < 4)
		//	ImGui::SetNextTreeNodeOpen(true);
        if (ImGui::TreeNode(res.name))
        {
            ImGui::SameLine();
            ImGui::Text("%.3f", res.milliseconds);

            drawMeasureImgui(res.childs);
            ImGui::TreePop();
        }
        else
        {
            ImGui::SameLine();
            ImGui::Text("%.3f", res.milliseconds);
        }
    }
	--DebugMeasureLevel;
}

static float rotY = 0.0f;
static float pos = 0.0f;

bool Engine::render()
{
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    engine::Device& device = m_renderSetup->device();
    CPU_MARKER(device.api(), "Render");

    //LOG("Starting new frame");

    /*{
        auto cmd = device.createCommandList("noh");
        cmd.clearRenderTargetView(m_renderSetup->currentRTV(), { 1.0f, 0.0f, 0.0f, 1.0f });
        m_renderSetup->submit(cmd);
        m_renderSetup->present();
        m_renderSetup->window().processMessages();
        return;
    }*/



    float deltaTime = delta();

#if 0
    auto root = m_scene->root()->child(1);
	if (root)
	{
        auto temp1 = root->child(5);
        if (temp1)
        {
            auto temp2 = temp1->child(1);
            if (temp2)
            {
                auto ciri = temp2->child(0);
                if (ciri)
                {
                    auto ciriTrans = ciri->getComponent<Transform>();
                    //ciriTrans->position({ 0.0f, 2.0f, std::sin(static_cast<float>(pos) / 60.0f)+11.0f });
                    ciriTrans->position({ 0.0f, 0.0f, -70.0f + (std::sin(static_cast<float>(pos)) * 70.0f) });
                    //ciriTrans->rotation(Quaternionf::fromEulerAngles(0.0f, rotY, 0.0f));
                    pos += 0.05f;
                    rotY += -50.0f;
                }
            }
        }
	}
#endif

    ImGui::SetCurrentContext(m_imguiContext.get());

    // Start imgui
    m_imguiRenderer->startFrame(device, deltaTime);

    auto& flatScene = m_scene->flatten(m_simulating, deltaTime);
    
    if ((flatScene.nodes.size() == 0) &&
        (flatScene.alphaclippedNodes.size() == 0) &&
        (flatScene.cameras.size() == 0))
    {
        ImGui::Render();
        return false;
    }

    {
        CPU_MARKER(m_renderSetup->device().api(), "Cpu/Gpu refresh");
        for (auto&& node : flatScene.nodes)
        {
            if (node.rigidBody && node.rigidBody->body())
            {
                if (std::find(
                    m_addedRigidBodies.begin(),
                    m_addedRigidBodies.end(),
                    static_cast<void*>(node.rigidBody->body().get())) == m_addedRigidBodies.end())
                {
                    m_dynamicsWorld->addRigidBody(node.rigidBody->body().get());
                    m_addedRigidBodies.emplace_back(node.rigidBody->body().get());
                }
            }

            if (node.mesh)
            {
                node.mesh->cpuRefresh(device);
                node.mesh->gpuRefresh(device);
            }

            if (node.material)
            {
                node.material->cpuRefresh(device);
                node.material->gpuRefresh(device);
            }

            //if (node.objectId == m_lastPickedObject)
            if(node.mesh->meshBuffer().modelAllocations && node.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex == static_cast<size_t>(m_lastPickedObject))
            {
                m_selectedObject = m_lastPickedObject;
                m_lastPickedObject = -1;
                m_lastPickedNode = m_scene->find(node.objectId);
            }
        }
        for (auto&& node : flatScene.alphaclippedNodes)
        {
            if (node.mesh)
            {
                node.mesh->cpuRefresh(device);
                node.mesh->gpuRefresh(device);
            }

            if (node.material)
            {
                node.material->cpuRefresh(device);
                node.material->gpuRefresh(device);
            }
            if (node.mesh->meshBuffer().modelAllocations && node.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex == static_cast<size_t>(m_lastPickedObject))
            {
                m_selectedObject = m_lastPickedObject;
                m_lastPickedObject = -1;
                m_lastPickedNode = m_scene->find(node.objectId);
            }
        }

        for (auto&& node : flatScene.transparentNodes)
        {
            if (node.mesh)
            {
                node.mesh->cpuRefresh(device);
                node.mesh->gpuRefresh(device);
            }

            if (node.material)
            {
                node.material->cpuRefresh(device);
                node.material->gpuRefresh(device);
            }
            if (node.mesh->meshBuffer().modelAllocations && node.mesh->meshBuffer().modelAllocations->subMeshInstance->instanceData.modelResource.gpuIndex == static_cast<size_t>(m_lastPickedObject))
            {
                m_selectedObject = m_lastPickedObject;
                m_lastPickedObject = -1;
                m_lastPickedNode = m_scene->find(node.objectId);
            }
        }

		for (auto&& terrain : flatScene.terrains)
		{
			terrain->update(device);
		}
    }

    flatScene.selectedObject = m_selectedObject;

    if (flatScene.selectedCamera != -1 && flatScene.cameras.size() > 0)
    {
        CPU_MARKER(m_renderSetup->device().api(), "Update camera and inputs");
        m_cameraInput->setCamera(flatScene.cameras[flatScene.selectedCamera]);
        m_inputManager->setCamera(flatScene.cameras[flatScene.selectedCamera].get());

        if (m_cameraSizeRefresh)
        {
            for (auto&& camera : flatScene.cameras)
            {
                //camera->width(m_renderSetup->window().width());
                //camera->height(m_renderSetup->window().height());
				camera->width(m_virtualResolution.x);
				camera->height(m_virtualResolution.y);
            }
            m_cameraSizeRefresh = false;
        }

        for (auto&& camera : flatScene.cameras)
        {
            camera->cpuRefresh(device);
            camera->gpuRefresh(device);
        }

        if (m_cameraInputActive)
        {
            m_cameraInput->update(deltaTime);
        }
        else
        {
            for(size_t i = 1; i < flatScene.cameras.size(); ++i)
            { 
                m_cameraInput->setCamera(flatScene.cameras[i]);
                m_cameraInput->position(flatScene.cameras[i]->position());
                m_cameraInput->rotation(flatScene.cameras[i]->rotation().toEulerAngles());
                m_cameraInput->apply(false);
            }
            m_cameraInput->setCamera(flatScene.cameras[flatScene.selectedCamera]);
            m_cameraInput->position(flatScene.cameras[flatScene.selectedCamera]->position());
            m_cameraInput->rotation(flatScene.cameras[flatScene.selectedCamera]->rotation().toEulerAngles());
            m_cameraInput->apply(false);
        }
    }

    m_inputManager->update();

    flatScene.lightData = m_lightData;

    if (m_updateEnvironmentOnNextFrame >= 0)
        --m_updateEnvironmentOnNextFrame;

    //LOG("STARTING NEW FRAME");
    auto cmd = device.createCommandList("Main frame cmd list");

    {
        CPU_MARKER(cmd.api(), "Stream resources");
        GPU_MARKER(cmd, "Stream resources");
        device.modelResources().streamResources(cmd);
    }

    {
        CPU_MARKER(cmd.api(), "Render");
        GPU_MARKER(cmd, "Render");

        {
            CPU_MARKER(cmd.api(), "Compute changed world areas");
            GPU_MARKER(cmd, "Compute changed world areas");

            m_viewportRenderer->shadowRenderer().computeChangedAreas(cmd);
        }

        {
            CPU_MARKER(cmd.api(), "Compute shadow updates");
            GPU_MARKER(cmd, "Compute shadow updates");
        }

        // Clear render targets
        clear(cmd);

        // start imguizmo (ui widgets)
        //ImGuizmo::BeginFrame();

        // update environment cubemap
#if 1
        if (flatScene.selectedCamera != -1 && flatScene.cameras.size() > 0 && flatScene.cameras[flatScene.selectedCamera])
            updateEnvironmentCubemap(device, cmd, *flatScene.cameras[flatScene.selectedCamera], false);

#if 1
        for (auto&& probe : flatScene.probes)
        {
            if (probe->update(device, cmd, flatScene))
            {
                if ((flatScene.selectedCamera != -1) &&
                    (flatScene.selectedCamera < static_cast<int>(flatScene.cameras.size())) &&
                    (flatScene.cameras[flatScene.selectedCamera]))
                {
                    Camera& selectedCamera = *flatScene.cameras[flatScene.selectedCamera];
                    selectedCamera.environmentMap(probe->cubemap());
                    selectedCamera.environmentBrdfLUT(probe->brdf());
                    selectedCamera.environmentIrradiance(probe->irradiance());
                    selectedCamera.environmentSpecular(probe->specular());
                }
            }
        }
#endif

        // lighting update
        updateLighting(cmd, flatScene);

        
        // render viewport
        m_viewportRenderer->render(cmd, flatScene, m_renderSetup->currentRTV(), 
            m_cameraInput->mousePosition().first,
            m_cameraInput->mousePosition().second);

        // render debug view
        renderDebugView(cmd);

        // render frame times
#if 1
        {
            if (*m_viewportRenderer->measuresEnabled())
			//if(true)
            {
                if (ImGui::Begin("Gpu Measures", m_viewportRenderer->measuresEnabled(), ImGuiWindowFlags_AlwaysAutoResize))
                {
                    drawMeasureImgui(m_lastFrameResults);
                }
                ImGui::End();
            }

            // fps
            /* {
                bool fpsVisible = true;
                ImGui::SetNextWindowContentSize({ 250.0f, 100.0f });
                if (ImGui::Begin("Gpu Stats", &fpsVisible,
                    ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoNav |
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_NoInputs |
                    ImGuiWindowFlags_NoBackground))
                {
                    auto windowSize = ImGui::GetWindowSize();
                    ImGui::SetWindowPos(ImVec2{ 
                        (static_cast<float>(m_renderSetup->window().width()) / 2.0f) - (windowSize.x / 2.0f), 0.0f });

                    float gpuFps = static_cast<float>(m_gpuFrameMicroseconds) / 1000.0f;
                    float cpuFps = static_cast<float>(m_cpuFrameMicroseconds) / 1000.0f;
                    ImGui::Text("GPU FPS: %f ms (%i)", gpuFps, static_cast<int>(1000.0f / gpuFps));
                    ImGui::Text("CPU FPS: %f ms (%i)", cpuFps, static_cast<int>(1000.0f / cpuFps));
                }
                ImGui::End();
            }*/

            bool bufferStatsVisible = *m_viewportRenderer->bufferStatsEnabled();
            bool cullingStatsVisible = *m_viewportRenderer->cullingStatsEnabled();
            bool statsVisible = bufferStatsVisible || cullingStatsVisible;
            if (statsVisible)
			//if(true)
            {
                ImGui::SetNextWindowContentSize({ 550.0f, 400.0f });
                if (ImGui::Begin("Gpu Stats", &statsVisible,
                    ImGuiWindowFlags_AlwaysAutoResize | 
                    ImGuiWindowFlags_NoNav | 
                    ImGuiWindowFlags_NoDecoration | 
                    ImGuiWindowFlags_NoInputs |
                    ImGuiWindowFlags_NoBackground))
                {
                    auto windowSize = ImGui::GetWindowSize();
                    ImGui::SetWindowPos(ImVec2{ static_cast<float>(m_renderSetup->window().width()) - windowSize.x, 0.0f });

                    auto lineTextList = [](const engine::vector<const char*>& data, const engine::vector<float>& widths)
                    {
                        for (int i = 0; i < data.size(); ++i)
                        {
                            ImGui::Text(data[i]);
                            if(i != data.size()-1)
                                ImGui::SameLine(widths[i]);
                        }
                    };

                    engine::vector<float> widths = { 
                        85.0f, 
                        85.0f * 2.0f, 
                        85.0f * 3.0f, 
                        85.0f * 4.0f,
                        85.0f * 5.0f + 60.0f };
                    if(bufferStatsVisible)
                        lineTextList({"", "inUse", "total", "free", "allocation MB", "inUse %"}, widths);
                    else
                        lineTextList({ "", "", "", "", "", "" }, widths);

                    if (bufferStatsVisible)
                    {
                        auto& gpuBuffers = m_renderSetup->device().modelResources().gpuBuffers();

                        auto& vertex = gpuBuffers.vertexDataAllocator();
                        ImGui::Text("vertex"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", vertex.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", vertex.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", vertex.elements() - vertex.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", vertex.usedElements() * vertex.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(vertex.usedElements()) / static_cast<float>(vertex.elements()) * 100.0f);

                        auto& uv = gpuBuffers.uvDataAllocator();
                        ImGui::Text("uv"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", uv.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", uv.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", uv.elements() - uv.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", uv.usedElements() * uv.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(uv.usedElements()) / static_cast<float>(uv.elements()) * 100.0f);

                        auto& index = gpuBuffers.indexAllocator();
                        ImGui::Text("index"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", index.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", index.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", index.elements() - index.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", index.usedElements() * index.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(index.usedElements()) / static_cast<float>(index.elements()) * 100.0f);

                        auto& adjacency = gpuBuffers.adjacencyAllocator();
                        ImGui::Text("adjacency"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", adjacency.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", adjacency.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", adjacency.elements() - adjacency.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", adjacency.usedElements() * adjacency.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(adjacency.usedElements()) / static_cast<float>(adjacency.elements()) * 100.0f);

                        auto& cluster = gpuBuffers.clusterDataAllocator();
                        ImGui::Text("cluster"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", cluster.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", cluster.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", cluster.elements() - cluster.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", cluster.usedElements() * cluster.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(cluster.usedElements()) / static_cast<float>(cluster.elements()) * 100.0f);

                        auto& submesh = gpuBuffers.subMeshDataAllocator();
                        ImGui::Text("submesh"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", submesh.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", submesh.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", submesh.elements() - submesh.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", submesh.usedElements() * submesh.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(submesh.usedElements()) / static_cast<float>(submesh.elements()) * 100.0f);

                        auto& lod = gpuBuffers.lodAllocator();
                        ImGui::Text("lod"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", lod.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", lod.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", lod.elements() - lod.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", lod.usedElements() * lod.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(lod.usedElements()) / static_cast<float>(lod.elements()) * 100.0f);

                        auto& instance = gpuBuffers.instanceDataAllocator();
                        ImGui::Text("instance"); ImGui::SameLine(widths[0]);
                        ImGui::Text("%zu", instance.usedElements()); ImGui::SameLine(widths[1]);
                        ImGui::Text("%zu", instance.elements()); ImGui::SameLine(widths[2]);
                        ImGui::Text("%zu", instance.elements() - instance.usedElements()); ImGui::SameLine(widths[3]);
                        ImGui::Text("%zu", instance.usedElements() * instance.elementSizeBytes() / 1024 / 1024); ImGui::SameLine(widths[4]);
                        ImGui::Text("%.2f %%", static_cast<float>(instance.usedElements()) / static_cast<float>(instance.elements()) * 100.0f);

                        ImGui::Separator();
                    }

                    if (cullingStatsVisible)
					//if(true)
                    {
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster frustum cull clusters in"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterFrustumCull_ClustersIn);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster frustum cull GBuffer clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterFrustumCull_ClustersOutGBuffer);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster frustum cull AlphaClip clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterFrustumCull_ClustersOutAlphaClipped);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster frustum cull Transparent clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterFrustumCull_ClustersOutTransparent);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster frustum cull Terrain clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterFrustumCull_ClustersOutTerrain);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand GBuffer indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandGBufferIndexes_IndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw GBuffer depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_Triangles);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw GBuffer depth draw count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_Count);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand AlphaClip indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandAlphaClipIndexes_IndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw AlphaClip depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawAlphaClip_Triangles);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw AlphaClip depth draw count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawAlphaClip_Count);
						
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand Transparent indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandTransparentIndexes_IndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Transparent depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawTransparent_Triangles);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Transparent depth draw count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawTransparent_Count);

						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand Terrain indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandTerrainIndexes_IndexesOut);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Terrain depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawTerrain_Triangles);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Terrain depth draw count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawTerrain_Count);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text(""); ImGui::SameLine(widths[4]); ImGui::Text("");

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Instance frustum instances in"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().instanceFrustum_InstancesIn);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Instance frustum instances out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().instanceFrustum_InstancesPassed);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Cluster expansion clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().clusterExpansion_ClustersOut);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Occlusion culling all clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().occlusionCulling_out_All);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Occlusion culling GBuffer clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().occlusionCulling_out_NotYetDrawnDepth);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Occlusion culling AlphaClip clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().occlusionCulling_out_AlphaClipped);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Occlusion culling Transparent clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().occlusionCulling_out_Transparent);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Occlusion culling Terrain clusters out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().occlusionCulling_out_Terrain);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand GBuffer indexes indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandIndexes_NotYetDrawnDepthIndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw GBuffer depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_Triangles2);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw GBuffer depth count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_Count2);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand AlphaClip indexes indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandIndexes_AlphaClippedIndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw AlphaClip depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_AlphaClippedTriangles);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw AlphaClip depth count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_AlphaClippedCount);

                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand Transparent indexes indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandIndexes_TransparentIndexesOut);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Transparent depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_TransparentTriangles);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Transparent depth count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_TransparentCount);

						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Expand Terrain indexes indexes out"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().expandIndexes_TerrainIndexesOut);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Terrain depth triangles"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_TerrainTriangles);
						ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Draw Terrain depth count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", m_viewportRenderer->getStatistics().drawDepth_TerrainCount);
						
						ImGui::Separator();
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Combined draw count"); ImGui::SameLine(widths[4]); ImGui::Text("%u", 
                            m_viewportRenderer->getStatistics().drawDepth_Count +
                            m_viewportRenderer->getStatistics().drawAlphaClip_Count +
                            m_viewportRenderer->getStatistics().drawTransparent_Count +
                            m_viewportRenderer->getStatistics().drawDepth_Count2 +
                            m_viewportRenderer->getStatistics().drawDepth_AlphaClippedCount +
                            m_viewportRenderer->getStatistics().drawDepth_TransparentCount +
							m_viewportRenderer->getStatistics().drawDepth_TerrainCount);
                        ImGui::Text(""); ImGui::SameLine(widths[1]); ImGui::Text("Combined triangle count"); ImGui::SameLine(widths[4]); ImGui::Text("%u",
                            m_viewportRenderer->getStatistics().drawDepth_Triangles +
                            m_viewportRenderer->getStatistics().drawAlphaClip_Triangles +
                            m_viewportRenderer->getStatistics().drawTransparent_Triangles +
                            m_viewportRenderer->getStatistics().drawDepth_Triangles2 +
                            m_viewportRenderer->getStatistics().drawDepth_AlphaClippedTriangles +
                            m_viewportRenderer->getStatistics().drawDepth_TransparentTriangles +
							m_viewportRenderer->getStatistics().drawDepth_TerrainTriangles);
                    }
                }
                ImGui::End();
            }
        }

        //m_lastFrameResults

        // render log window
        if(*m_viewportRenderer->logEnabled())
            m_logWindow.render(
                m_renderSetup->window().width(), 
                m_renderSetup->window().height());
#endif

        // render debug menu
        /*m_debugMenu.render(
            m_renderSetup->window().width(), 
            m_renderSetup->window().height());*/

        // draw gizmos
        /*if (flatScene.camera)
        {
            auto viewMat = flatScene.camera->viewMatrix().transpose();
            auto projMat = flatScene.camera->projectionMatrix().transpose();


            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::Enable(true);
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            ImGuizmo::Manipulate(
                &viewMat.m00,
                &projMat.m00,
                ImGuizmo::OPERATION::ROTATE,
                ImGuizmo::MODE::WORLD,
                &id.m00);


            ImGuizmo::DrawCube(
                &viewMat.m00,
                &projMat.m00,
                &id.m00);
        }*/
#endif
        // draw imgui stuff
        {
            CPU_MARKER(cmd.api(), "Imgui");
            GPU_MARKER(cmd, "Imgui");

            m_imguiRenderer->render(flatScene);

            // Stop imgui
            m_imguiRenderer->endFrame(device, m_renderSetup->currentRTV(), m_viewportRenderer->dsv(), cmd);
        }
    }

    auto instanceCount = device.modelResources().instanceCount();
    if(instanceCount > 0)
    {
        if (!m_cycleBufferView)
        {
            m_cycleBufferView = device.createBufferUAV(device.modelResources().gpuBuffers().instanceTransformOwner());
        }
        CPU_MARKER(cmd.api(), "Cycle transforms");
        GPU_MARKER(cmd, "Cycle transforms");
        m_cycleTransforms->cs.transformCount.x = static_cast<uint32_t>(device.modelResources().instanceCount());
        m_cycleTransforms->cs.historyBuffer = m_cycleBufferView;
        cmd.bindPipe(*m_cycleTransforms);
        cmd.dispatch(roundUpToMultiple(instanceCount, 64) / 64, 1, 1);
    }

    // execute command list and present
    {
        cmd.transition(m_renderSetup->currentRTV(), ResourceState::Present);
    }

    //LOG("Submitting new frame");
    if (m_mode == EngineMode::OwnThread || m_mode == EngineMode::OwnThreadNoPresent)
        m_renderSetup->submit(cmd);
    else
        m_renderSetup->submitBlocking(cmd);
    //    m_storedCommandList = std::move(cmd);

    LARGE_INTEGER stop;
    QueryPerformanceCounter(&stop);

    //LOG("Presenting new frame");
    //if (m_mode == EngineMode::OwnThread || m_mode == EngineMode::OwnThreadNoPresent)
        m_renderSetup->present(true);
    //LOG("Picked object id: %u", m_modelRenderer->pickedObject(device));
    
    //LOG("Getting query results");
    // pump query buffers
    auto queryResults = m_renderSetup->device().fetchQueryResults();
    if (queryResults.size() > 0)
    {
        // we know render will be the index 1 item
        if (queryResults[0].size() > 1)
        {
            auto& temp = queryResults[0][1];
            auto gpufreq = m_renderSetup->device().queue().timeStampFrequency();
            m_gpuFrameMicroseconds = ((temp.startTick - m_lastGpuFrameStart) * 1000000) / gpufreq;
            m_lastGpuFrameStart = temp.startTick;
        }

		engine::QueryResultTicks cpuResult;
        LARGE_INTEGER elapsedMicroseconds;
        elapsedMicroseconds.QuadPart = stop.QuadPart - start.QuadPart;
        elapsedMicroseconds.QuadPart *= 1000;
        cpuResult.milliseconds = static_cast<float>(static_cast<double>(elapsedMicroseconds.QuadPart) / static_cast<double>(freq.QuadPart));
        cpuResult.name = "CPU Time";

        m_lastFrameResults = queryResults.back();
        m_lastFrameResults.insert(m_lastFrameResults.begin(), cpuResult);
    }

    if (m_mode == EngineMode::OwnThreadNoPresent)
        m_frameCapturer->captureFrame(m_renderSetup->prevRTVSRV());

    // process window messages
    //LOG("Processing messages");
    if (m_mode == EngineMode::OwnThread)
        m_renderSetup->window().processMessages();

	if (RefreshPipelines > 0)
	{
		--RefreshPipelines;
	}
    //LOG("FRAME DONE");

    auto timeNow = std::chrono::high_resolution_clock::now();
    m_cpuFrameMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(timeNow - m_lastFrameTime).count();
    m_lastFrameTime = timeNow;
    

	return true;
}

void Engine::clear(engine::CommandList& cmd)
{
    CPU_MARKER(cmd.api(), "Clear render targets");
    GPU_MARKER(cmd, "Clear render targets");
    cmd.clearRenderTargetView(m_renderSetup->currentRTV(), { 0.0f, 0.0f, 0.0f, 0.0f });
}

void Engine::updateLighting(engine::CommandList& cmd, engine::FlatScene& flatScene)
{
    CPU_MARKER(cmd.api(), "Update lighting");
    GPU_MARKER(cmd, "Update lighting");
    //m_defaultMaterial->cpuRefresh(device);
    //m_defaultMaterial->gpuRefresh(m_renderSetup->device());
    flatScene.lightData->updateLightInfo(m_renderSetup->device(), cmd, flatScene.lights);
    if (m_lightData->changeHappened())
    {
        m_scene->root()->invalidate();
    }
}

void Engine::updateEnvironmentCubemap(engine::Device& device, engine::CommandList& cmd, engine::Camera& camera, bool force)
{
    CPU_MARKER(cmd.api(), "Update environment cubemap");
    if (camera.environmentMap().valid())// && )
    {
        if (camera.environmentMapPath() != m_lastEnvironmentMap || force)
        {
            // only one slice. we need equirect mapping to cubemap
            if (camera.environmentMap().texture().arraySlices() == 1)
            {
				m_viewportRenderer->cubemapRenderer().cubemap(TextureSRV{});
                m_viewportRenderer->cubemapRenderer().createCubemapFromEquirect(device, camera.environmentMap());
                m_viewportRenderer->cubemapRenderer().createIrradianceCubemap(device, m_viewportRenderer->cubemapRenderer().cubemap(), cmd);
                m_viewportRenderer->cubemapRenderer().prefilterConvolution(device, m_viewportRenderer->cubemapRenderer().cubemap(), cmd);
                m_viewportRenderer->cubemapRenderer().brdfConvolution(device, cmd);
            }
            else
            {
                m_viewportRenderer->cubemapRenderer().cubemap(camera.environmentMap());
                m_viewportRenderer->cubemapRenderer().createIrradianceCubemap(device, camera.environmentMap(), cmd);
                m_viewportRenderer->cubemapRenderer().prefilterConvolution(device, camera.environmentMap(), cmd);
                m_viewportRenderer->cubemapRenderer().brdfConvolution(device, cmd);
            }
            
            camera.environmentBrdfLUT(m_viewportRenderer->cubemapRenderer().brdfConvolution());
            camera.environmentIrradiance(m_viewportRenderer->cubemapRenderer().irradiance());
            camera.environmentSpecular(m_viewportRenderer->cubemapRenderer().prefilteredEnvironmentMap());

            m_lastEnvironmentMap = camera.environmentMapPath();
            m_updateEnvironmentOnNextFrame = -1;
        }
    }
}

void Engine::renderDebugView(engine::CommandList& /*cmd*/)
{
    /*CPU_MARKER(cmd.api(), "Render debug view");
    GPU_MARKER(cmd, "Render debug view");
    m_debugViewer->render(
		cmd,
		m_renderSetup->currentRTV(),
        m_viewportRenderer->ssrResult()
    );*/

    /*m_debugViewer->render(
        m_renderSetup->device(),
        m_renderSetup->currentRTV(),
        cmd,
        m_modelRenderer->ssaoSRV()
        //m_shadowRenderer->shadowMap()
    );*/

    /*m_debugViewer->render(
        m_renderSetup->device(),
        m_renderSetup->currentRTV(),
        cmd,
        m_renderCubemap->irradiance()
    );*/

    /*m_debugViewer->render(
        m_renderSetup->device(),
        m_renderSetup->currentRTV(),
        cmd,
        m_renderCubemap->brdfConvolution()
        //m_shadowRenderer->shadowMap()
    );*/

    /*m_debugViewer->render(
        m_renderSetup->device(),
        m_renderSetup->currentRTV(),
        cmd,
        m_renderCubemap->posx()
        //m_shadowRenderer->shadowMap()
    );*/

    


    /*m_debugViewer->render(
        m_renderSetup->device(),
        m_renderSetup->currentRTV(),
        cmd,
        m_modelTransparentRenderer->colorAccumulateSRV()
        //m_shadowRenderer->shadowMap()
    );*/
}

void Engine::moveToNewWindowBegin()
{
	std::lock_guard<std::mutex> lock(m_engineMutex);
	m_requestEngineStallBeginEnd = true;
}

void Engine::moveToNewWindowEnd(engine::shared_ptr<platform::Window> newWindow)
{
	bool engineStalled = false;
	while (!engineStalled)
	{
		{
			std::lock_guard<std::mutex> lock(m_engineMutex);
			engineStalled = m_engineStalled;
		}
		m_window->processMessages();
	}

	m_window = newWindow;
	m_renderSetup->window(newWindow);
	m_renderSetup->device().window(newWindow);
	m_renderSetup->recreateSwapChain();

	{
		std::lock_guard<std::mutex> lock(m_engineMutex);
		m_requestEngineStallBeginEnd = true;
	}
}

void Engine::refresh(bool updateVirtualResolution)
{
#ifdef _DURANGO
	return;
#endif

	RefreshPipelines = 10;

    m_renderSetup->device().shutdown();
    m_renderSetup->device().waitForIdle();

    //std::this_thread::sleep_for(std::chrono::milliseconds(100));

    m_window->refreshSize();

    if (m_renderSetup->device().width() == 0 || m_renderSetup->device().height() == 0)
        return;

	if(updateVirtualResolution)
		m_virtualResolution = Vector2<int>{ m_renderSetup->device().width(), m_renderSetup->device().height() };
	
    auto weakChain = m_renderSetup->device().currentSwapChain();
    auto swapChain = weakChain.lock();
    if (swapChain)
    {
        m_renderSetup->releaseSwapChainSRVs();
        swapChain->resize(m_renderSetup->device() , { static_cast<uint32_t>(m_renderSetup->device().width()), static_cast<uint32_t>(m_renderSetup->device().height()) });
    }
    m_renderSetup->createSwapChainSRVs();
	
    
    m_viewportRenderer->refresh(m_virtualResolution);
    m_cameraSizeRefresh = true;

    //m_camera->width(m_window->width());
    //m_camera->height(m_window->height());
	
    m_renderSetup->device().shutdown(false);

}

CameraTransform Engine::getCameraTransform() const
{
    return { m_cameraInput->position(), m_cameraInput->rotation() };
}

void Engine::setCameraTransform(const CameraTransform& transform)
{
    m_cameraInput->position(transform.position);
    m_cameraInput->rotation(transform.rotation);
    m_cameraInputActive = true;
}

void Engine::onMouseMove(int x, int y)
{
    m_renderSetup->window().mouseCursor(platform::MouseCursor::Arrow);


    m_imguiRenderer->onMouseMove(x, y);
    if (!m_imguiRenderer->usesMouse())
    {
        m_cameraInput->onMouseMove(x, y);
    }

	if (m_mouseLeftDown)
	{
		m_viewportRenderer->setSSRDebugMousePosition(x, y);
	}
}

void Engine::onMouseDown(MouseButton button, int x, int y)
{
	if (button == MouseButton::Left)
	{
		auto debugPhase = m_viewportRenderer->ssrDebugPhase();
		debugPhase += 1;
		debugPhase %= 20;
		m_viewportRenderer->ssrDebugPhase(debugPhase);
		m_mouseLeftDown = true;
	}
    //LOG("onMouseDown: %i, %i", x, y);
    m_imguiRenderer->onMouseDown(button, x, y);
    if (!m_imguiRenderer->usesMouse())
    {
        m_cameraInput->onMouseDown(button, x, y);

        if (button == MouseButton::Left)
        {
            auto pickObjectId = static_cast<int64_t>(m_viewportRenderer->pickedObject(m_renderSetup->device()));
            m_viewportRenderer->setSelectedObject(pickObjectId);
            m_lastPickedObject = pickObjectId;
        }
    }

    
}

void Engine::onMouseUp(MouseButton button, int x, int y)
{
	if (button == MouseButton::Left)
		m_mouseLeftDown = false;

    //LOG("onMouseUp: %i, %i", x, y);
    m_imguiRenderer->onMouseUp(button, x, y);
    if (!m_imguiRenderer->usesMouse())
    {
        m_cameraInput->onMouseUp(button, x, y);
    }
}

void Engine::onMouseDoubleClick(MouseButton button, int x, int y)
{
    m_imguiRenderer->onMouseDoubleClick(button, x, y);
    if (!m_imguiRenderer->usesMouse())
    {
        m_cameraInput->onMouseDoubleClick(button, x, y);
    }
}

void Engine::onMouseWheel(int x, int y, int delta)
{
	if (!m_imguiRenderer->usesMouse())
	{
		m_cameraInput->onMouseWheel(x, y, delta);
	}
	else
		m_imguiRenderer->onMouseWheel(x, y, delta);
}

void Engine::shutdown()
{
    m_renderSetup->device().shutdown();
    m_renderSetup->device().waitForIdle();
    
    m_cycleBufferView = BufferUAVOwner();
    m_cycleTransforms.reset(nullptr);
    m_scene->clear(true);
	m_lastPickedNode = nullptr;
    m_frameCapturer = nullptr;

	m_lightData = nullptr;
	//m_camera = nullptr;

    m_debugViewer.reset(nullptr);
	m_viewportRenderer.reset(nullptr);
	m_imguiRenderer.reset(nullptr);
	m_cameraInput.reset(nullptr);

    m_renderSetup->device().resourceCache().clear();
	m_renderSetup->shutdown();

    
    
    
    m_renderSetup.reset(nullptr);
}

void Engine::onKeyDown(engine::Key key, const engine::ModifierState& modifierState)
{
    m_cameraInput->onKeyDown(key, modifierState);
}

void Engine::onKeyUp(engine::Key key, const engine::ModifierState& modifierState)
{
    m_cameraInput->onKeyUp(key, modifierState);
}

void Engine::cameraInputActive(bool active)
{
    m_cameraInputActive = active;
}
