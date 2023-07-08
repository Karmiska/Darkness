#pragma once

#include "tools/Debug.h"
#include "engine/ComponentRegister.h"
#include "tools/Serialization.h"
#include "containers/string.h"
#include "containers/memory.h"
#include <functional>

namespace engine
{
    enum class ButtonPush
    {
        NotPressed,
        Pressed
    };
    enum class ButtonToggle
    {
        NotPressed,
        Pressed
    };

    class SceneNode;
    class Property;
    class EngineComponent : public ComponentRegister
    {
    protected:
        engine::string m_name;

        template <class T>
        engine::shared_ptr<T> getComponent()
        {
            auto components = getComponents();
            for (auto& component : components)
            {
                if (std::dynamic_pointer_cast<typename std::decay<T>::type>(component))
                {
                    return std::dynamic_pointer_cast<typename std::decay<T>::type>(component);
                }
            }
            return nullptr;
        }

        engine::vector<engine::shared_ptr<EngineComponent>> getComponents();

        friend class Property;
        virtual void onValueChanged() {};

        friend class SceneNode;
        virtual void onUpdate(float /*deltaSeconds*/) {};
    public:
        virtual ~EngineComponent() {};

        engine::string name() const;
        void name(const engine::string& name);

        void parentNode(SceneNode* node);
        SceneNode* parentNode() const;

        virtual void start() {};

        void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);

        virtual engine::shared_ptr<engine::EngineComponent> clone() const = 0;
    private:
        SceneNode* m_parentNode = nullptr;
    };
}
