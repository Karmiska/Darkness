#include "Application.h"
#include "FollowCamera.h"
#include "CharacterInputManager.h"
#include <chrono>
#include <random>

#ifndef _DURANGO
#include "engine/resources/ResourceDropHandler.h"
#include "engine/resources/ResourceHost.h"
#endif

using namespace engine;
using namespace platform;

namespace application
{
#if 1
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(-20.0f, 20.0f);
    float randomFloat()
    {
        return static_cast<float>(distribution(generator));
    }
#endif

    engine::shared_ptr<engine::SceneNode> duplicateNode(const SceneNode* srcnode)
    {
        auto node = engine::make_shared<SceneNode>();
        for (int i = 0; i < srcnode->componentCount(); ++i)
            node->addComponent(srcnode->component(i)->clone());

        for (int i = 0; i < srcnode->childCount(); ++i)
        {
            node->addChild(duplicateNode(srcnode->child(i).get()));
        }
        return node;
    };

    Application::Application()
        : m_window{ engine::make_shared<platform::Window>("Darkness", ScreenWidth, ScreenHeight) }
        , m_characterInputManager{ engine::make_shared<CharacterInputManager>(ScreenWidth, ScreenHeight) }
        , m_engine{ m_window, "", GraphicsApi::DX12, EngineMode::OwnThread, "GameApplication", nullptr, m_characterInputManager}
        , m_followCameraComponent{ nullptr }
    {
        m_window->setResizeCallback([&](int /*width*/, int /*height*/) { m_engine.refreshSize(); });
        m_window->setMouseCallbacks(
            [&](int x, int y) { m_engine.onMouseMove(x, y); },
            [&](engine::MouseButton btn, int x, int y) { m_engine.onMouseDown(btn, x, y); m_engine.cameraInputActive(true); },
            [&](engine::MouseButton btn, int x, int y) { m_engine.onMouseUp(btn, x, y); m_engine.cameraInputActive(false); },
            [&](engine::MouseButton btn, int x, int y) { m_engine.onMouseDoubleClick(btn, x, y); },
			[&](int x, int y, int delta) { m_engine.onMouseWheel(x, y, delta); }
        );
        m_window->setKeyboardCallbacks(
            [&](engine::Key key, engine::ModifierState modState) { m_engine.onKeyDown(key, modState); },
            [&](engine::Key key, engine::ModifierState modState) { m_engine.onKeyUp(key, modState); }
        );

        //m_engine.scene().loadFrom("C:\\Users\\aleks\\Documents\\TestDarknessProject\\content\\scenes\\empty");
        auto scenePath = engine::pathClean(engine::pathJoin(engine::getWorkingDirectory(), "..\\..\\data\\empty"));
        if(!engine::pathExists(scenePath))
            scenePath = engine::pathClean(engine::pathJoin(engine::getWorkingDirectory(), "..\\..\\..\\data\\empty"));
        if (!engine::pathExists(scenePath))
            scenePath = "C:\\Users\\aleks\\Documents\\TestDarknessProject\\content\\scenes\\ball2";
        m_engine.scene().loadFrom(scenePath);

        //auto dataRoot = engine::pathClean(engine::pathJoin(engine::pathExtractFolder(engine::getExecutableDirectory()), "..\\..\\..\\data"));
        //m_engine.scene().loadFrom(engine::pathJoin(dataRoot, "\\content\\scenes\\cave"));

#if 0
        m_engine.pauseEngine(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto sphere = m_engine.scene().root()->child(1);

        for(int i = 0; i < 10; ++i)
        {
            auto newNode = duplicateNode(sphere.get());
            newNode->getComponent<Transform>()->position(engine::Vector3f(randomFloat(), randomFloat(), randomFloat()));
            m_engine.scene().root()->addChild(newNode);
        }

        auto flatten = m_engine.scene().flatten(false, 0.0f);
        m_engine.pauseEngine(false);
#endif


        std::chrono::high_resolution_clock::time_point last = std::chrono::high_resolution_clock::now();
        bool res = true;

        auto cameraNode = m_engine.scene().find("Camera");
        auto mainCharacterNode = m_engine.scene().find("male");
        //ASSERT(cameraNode&& mainCharacterNode, "Failed to get character camera or character");

        //attachFollowCamera(cameraNode, mainCharacterNode);
        //attachInputManager(cameraNode, mainCharacterNode);
        
        while (res)
        {
            res = m_window->processMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void Application::attachFollowCamera(
        engine::shared_ptr<SceneNode> camera,
        engine::shared_ptr<SceneNode> character)
    {
        m_followCameraComponent = engine::make_shared<application::FollowCamera>();
        m_followCameraComponent->followThisNode(character);
        camera->addComponent(m_followCameraComponent);
    }

    void Application::attachInputManager(
        engine::shared_ptr<SceneNode> camera,
        engine::shared_ptr<SceneNode> character)
    {
        m_characterInputManager->setCharacterNode(character);
        m_characterInputManager->setCameraNode(camera);
        m_characterInputManager->setCameraRotateCallback([this](int rotateDirection) { this->m_followCameraComponent->rotateCamera(rotateDirection); });
    }
}
