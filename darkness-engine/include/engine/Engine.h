#pragma once

#include "components/ProbeComponent.h"
#include "engine/Scene.h"
#include "engine/CameraInput.h"
#include "components/Camera.h"
#include "rendering/Rendering.h"
#include "platform/window/Window.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/ImguiRenderer.h"
#include "engine/rendering/DebugView.h"
#include "engine/rendering/LogWindow.h"
#include "engine/rendering/ViewportRenderer.h"
#include "engine/rendering/DebugMenu.h"
#include "engine/RenderSetup.h"
#include "engine/sound/Sound.h"
#include "engine/FrameCpuCapturer.h"

#include "engine/primitives/Quaternion.h"
#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "engine/input/InputManager.h"
#ifndef _DURANGO
#include "engine/resources/ResourceHost.h"
#endif

#include "shaders/core/tools/CycleTransforms.h"

#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/PostprocessComponent.h"

#include "containers/memory.h"
#include "containers/string.h"
#include <chrono>
#include <thread>
#include <mutex>

extern int RefreshPipelines;

//#include "btBulletDynamicsCommon.h"

/*class AssetConvert
{
public:
    AssetConvert(const engine::shared_ptr<engine::Mesh>& source);

    engine::vector<engine::Vertex> target;
    engine::vector<uint32_t>       targetIndices;
};*/

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

struct ImGuiContext;

namespace engine
{
    struct QueryResult;
}

struct CameraTransform
{
    engine::Vector3f position;
    engine::Vector3f rotation;
};

class Engine
{
public:
    Engine(
        engine::shared_ptr<platform::Window> window,
        const engine::string& shaderRootPath,
        GraphicsApi api,
        EngineMode mode,
        const char* name,
        engine::shared_ptr<engine::Scene> scene = nullptr,
        engine::shared_ptr<InputManager> inputManager = nullptr);
    ~Engine();

	void moveToNewWindowBegin();
	void moveToNewWindowEnd(engine::shared_ptr<platform::Window> newWindow);

    engine::Scene& scene();

    void refreshSize();
	void loadScene(const engine::string& filePath);

    void onMouseMove(int x, int y);
    void onMouseDown(engine::MouseButton button, int x, int y);
    void onMouseUp(engine::MouseButton button, int x, int y);
    void onMouseDoubleClick(engine::MouseButton button, int x, int y);
	void onMouseWheel(int x, int y, int delta);

    void onKeyDown(engine::Key key, const engine::ModifierState& modifierState);
    void onKeyUp(engine::Key key, const engine::ModifierState& modifierState);

    void cameraInputActive(bool active);
    void resetCameraSize();
    void playClicked(bool value);

    engine::shared_ptr<engine::SceneNode> grabSelected();
    void setSelected(engine::shared_ptr<engine::SceneNode> node);

    bool stepEngine();

    CameraTransform getCameraTransform() const;
    void setCameraTransform(const CameraTransform& transform);

    CpuTexture grabRTV();

    EngineMode mode() const { return m_mode; }
    bool hasStoredCommandList() const { return m_storedCommandList.verify(); }
    engine::CommandList&& storedCommandList() { return std::move(m_storedCommandList); }
    engine::RenderSetup* renderSetup() { return m_renderSetup.get(); }

    void pauseEngine(bool val);
private:
    bool update();
    void shutdown();
    void refresh(bool updateVirtualResolution = true);

    EngineMode m_mode;
    engine::shared_ptr<platform::Window> m_window;
    engine::unique_ptr<engine::RenderSetup> m_renderSetup;
    engine::string m_shaderRootPath;
    engine::shared_ptr<engine::Scene> m_scene;
    engine::Vector2<int> m_virtualResolution;
    engine::unique_ptr<engine::ViewportRenderer> m_viewportRenderer;
    engine::shared_ptr<ImGuiContext> m_imguiContext;
    engine::unique_ptr<engine::ImguiRenderer> m_imguiRenderer;
    engine::unique_ptr<engine::DebugView> m_debugViewer;
    engine::unique_ptr<engine::Pipeline<engine::shaders::CycleTransforms>> m_cycleTransforms;
    engine::BufferUAVOwner m_cycleBufferView;
    engine::LogWindow m_logWindow;
    engine::shared_ptr<InputManager> m_inputManager;
    engine::DebugMenu m_debugMenu;
    engine::unique_ptr<engine::FrameCpuCapturer> m_frameCapturer;

    bool m_cameraSizeRefresh;

    engine::unique_ptr<engine::CameraInput> m_cameraInput;
    bool m_cameraInputActive;

    engine::unique_ptr<engine::MaterialComponent> m_defaultMaterial;
    engine::shared_ptr<engine::LightData> m_lightData;

    bool render();
    void clear(engine::CommandList& cmd);
    void updateEnvironmentCubemap(engine::Device& device, engine::CommandList& cmd, engine::Camera& camera, bool force = false);
    void updateLighting(engine::CommandList& cmd, engine::FlatScene& flatScene);
    void renderDebugView(engine::CommandList& cmd);

    float delta();
    std::chrono::high_resolution_clock::time_point m_lastUpdate;

    engine::shared_ptr<engine::Camera> camera();
    engine::string m_lastEnvironmentMap;
    //engine::shared_ptr<engine::Transform> m_cameraTransform;
    //engine::unique_ptr<engine::Camera> m_camera;

    // bullet
    engine::shared_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
    engine::shared_ptr<btCollisionDispatcher> m_dispatcher;
    engine::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;
    engine::shared_ptr<btSequentialImpulseConstraintSolver> m_solver;
    engine::shared_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

    engine::shared_ptr<void> m_collisionShapes;

    engine::vector<void*> m_addedRigidBodies;

    bool m_simulating;
    int64_t m_lastPickedObject;
    int64_t m_selectedObject;
    engine::shared_ptr<engine::SceneNode> m_lastPickedNode;

    engine::vector<engine::QueryResultTicks> m_lastFrameResults;

    int m_updateEnvironmentOnNextFrame;
	volatile bool m_engineRunning;
	bool m_engineExited;
	
	bool m_requestEngineStallBeginEnd;
	bool m_engineStalled;

	std::mutex m_engineMutex;
    engine::unique_ptr<std::thread> m_engineThread;
    
    
    bool engineRun();

    bool m_refreshSize;
	bool m_sceneLoadRequest;
	engine::string m_sceneToLoad;

	bool m_mouseLeftDown;
    //engine::Sound m_sound;

    std::chrono::steady_clock::time_point m_lastFrameTime;
    long long m_cpuFrameMicroseconds;
    uint64_t m_lastGpuFrameStart;
    long long m_gpuFrameMicroseconds;

    engine::CommandList m_storedCommandList;
};
