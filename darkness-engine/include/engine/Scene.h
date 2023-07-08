#pragma once

#include "engine/primitives/Vector3.h"
#include "engine/primitives/Matrix4.h"
#include "engine/primitives/Quaternion.h"
#include "engine/EngineComponent.h"
#include "plugins/PluginManager.h"
#include "tools/Property.h"
#include "tools/Serialization.h"
#include "tools/StringTools.h"
#include "containers/vector.h"
#include "containers/memory.h"
#include "containers/string.h"
#include <functional>
#include <map>
#include <stack>
#include <algorithm>
#include <sstream>

#include "components/Transform.h"
#include "components/MeshRendererComponent.h"
#include "components/MaterialComponent.h"
#include "components/LightComponent.h"
#include "components/Camera.h"
#include "components/PostprocessComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/CollisionShapeComponent.h"
#include "components/ProbeComponent.h"

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif
using namespace engine;
using namespace rapidjson;

#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

namespace engine
{
	class TerrainComponent;

    struct FlatSceneNode
    {
        Matrix4f transform;
        Matrix4f previousTransform;
        engine::shared_ptr<MeshRendererComponent> mesh;
        engine::shared_ptr<MaterialComponent> material;
        engine::shared_ptr<RigidBodyComponent> rigidBody;
        unsigned int objectId;
    };

    struct FlatSceneLightNode
    {
        Matrix4f transform;
        Vector3f position;
        Vector3f direction;
        float range;
        Quaternionf rotation;
        LightType type;
        float outerCone;
        float innerCone;
        bool shadowCaster;
        bool positionChanged;
        bool rotationChanged;
        engine::shared_ptr<LightComponent> light;
        engine::shared_ptr<SceneNode> node;
    };

    class LightData;
    struct FlatScene
    {
        engine::vector<FlatSceneNode> nodes;
        engine::vector<FlatSceneNode> transparentNodes;
        engine::vector<FlatSceneNode> alphaclippedNodes;

        engine::vector<engine::shared_ptr<Camera>> cameras;
        engine::vector<engine::shared_ptr<SceneNode>> cameraNodes;

        engine::vector<engine::shared_ptr<ProbeComponent>> probes;
        engine::vector<engine::shared_ptr<SceneNode>> probeNodes;

		engine::vector<engine::shared_ptr<TerrainComponent>> terrains;
		engine::vector<engine::shared_ptr<SceneNode>> terrainNodes;
        int selectedCamera;
        engine::shared_ptr<PostprocessComponent> postprocess;
        engine::vector<FlatSceneLightNode> lights;

        engine::shared_ptr<LightData> lightData;

        bool refreshed = false;
        int64_t selectedObject;
        void clear()
        {
            nodes.clear();
            transparentNodes.clear();
            alphaclippedNodes.clear();
            cameras.clear();
            cameraNodes.clear();
            probes.clear();
            probeNodes.clear();
			terrains.clear();
			terrainNodes.clear();
            postprocess = nullptr;
            lights.clear();
        }

        bool validScene()
        {
            return !(selectedCamera == -1 || 
                cameras.size() == 0 || 
                selectedCamera > static_cast<int>(cameras.size()) - 1 || 
                !cameras[selectedCamera]);
        }

        Camera& drawCamera()
        {
            return *cameras[0];
        }

        Camera& cullingCamera()
        {
            if (cameras.size() > 1)
                return *cameras[1];
            else
                return *cameras[0];
        }

        bool cameraDebugging() const
        {
            return cameras.size() > 1;
        }
    };

    class SceneNode : public std::enable_shared_from_this<SceneNode>
    {
    public:
        SceneNode(SceneNode* parent = nullptr);
        ~SceneNode();
        SceneNode(SceneNode&&) = default;
        SceneNode(const SceneNode&) = default;
        SceneNode& operator=(SceneNode&&) = default;
        SceneNode& operator=(const SceneNode&) = default;

        void addChild(engine::shared_ptr<SceneNode> child);
        void removeChild(engine::shared_ptr<SceneNode> child);

        const engine::shared_ptr<SceneNode> child(size_t index) const;
        engine::shared_ptr<SceneNode> child(size_t index);
        
        const engine::shared_ptr<SceneNode> operator[](size_t index) const;
        engine::shared_ptr<SceneNode> operator[](size_t index);

        void parent(SceneNode* parent);

        const SceneNode* parent() const;
        SceneNode* parent();

        void flatten(bool simulating, FlatScene& resultList, unsigned int& objectIndex, float deltaSeconds);

        size_t indexInParent() const;
        size_t childCount() const;

        size_t componentCount() const;
        engine::shared_ptr<EngineComponent> component(size_t index);
        const engine::shared_ptr<EngineComponent> component(size_t index) const;
        
        void addComponent(engine::shared_ptr<EngineComponent> component);
        void removeComponent(engine::shared_ptr<EngineComponent> component);

        void addComponent(TypeInstance&& component);

        engine::vector<TypeInstance>& components()
        {
            return m_componentinstances;
        }

        template <class T>
        engine::shared_ptr<T> getComponent()
        {
            for(auto& component : m_components)
            {
                if (std::dynamic_pointer_cast<typename std::decay<T>::type>(component))
                {
                    return std::dynamic_pointer_cast<typename std::decay<T>::type>(component);
                }
            }
            return nullptr;
        }

        engine::string name() const;
        SceneNode& name(const engine::string& name);

		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const;

		bool invalid() const;
		void invalidate();

		int64_t id() const;

        engine::shared_ptr<SceneNode> find(int64_t id);
        engine::shared_ptr<SceneNode> find(const engine::string& name);
        engine::vector<engine::shared_ptr<SceneNode>> path(int64_t id);

        const Matrix4f& combinedTransform() const { return m_combinedTransform; }
    private:
        engine::vector<engine::shared_ptr<SceneNode>> m_childs;
        engine::vector<engine::shared_ptr<EngineComponent>> m_components;
        engine::vector<TypeInstance> m_componentinstances;
        SceneNode* m_parent;
        engine::string m_name;

        engine::shared_ptr<Transform> m_transformComponent;
        Matrix4f m_combinedTransform;

        void recalculateCombinedTransform();

        bool m_invalid;
        int64_t m_id;

        void onUpdate(float deltaSeconds);
    };

    class Scene
    {
    public:
        Scene();

        FlatScene& flatten(bool simulating, float deltaSeconds, engine::shared_ptr<SceneNode> node = nullptr);

        template <class T>
        engine::shared_ptr<T> getComponent(engine::shared_ptr<SceneNode> node = nullptr)
        {
            if (!node)
                node = m_rootNode;

            auto component = node->getComponent<T>();
            if (component)
                return component;

            for (size_t i = 0; i < node->childCount(); ++i)
            {
                component = node->child(i)->getComponent<T>();
                if (component)
                    return component;
            }

            return nullptr;
        }

        const engine::shared_ptr<SceneNode> root() const;
        engine::shared_ptr<SceneNode>& root();

        void setRoot(engine::shared_ptr<SceneNode> rootNode)
        {
            m_rootNode = rootNode;
        }

        void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const
        {
            m_rootNode->serialize(writer);
        }

        void saveTo(const engine::string& filepath);
        void loadFrom(const engine::string& filepath);

        void clear(bool full = false);

        engine::shared_ptr<SceneNode> find(int64_t id);
        engine::shared_ptr<SceneNode> find(const engine::string& name);
        engine::vector<engine::shared_ptr<SceneNode>> path(int64_t id);

    private:
        FlatScene m_flatscene;
        engine::shared_ptr<SceneNode> m_rootNode;
    };

    extern int ident;
    engine::string identString();

    class SceneDeserialize : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, SceneDeserialize>
    {
    public:
        SceneDeserialize(engine::shared_ptr<SceneNode> scene)
            : m_scene{ scene }
            //, m_nodeCurrent{ scene }
            , m_componentCurrent{ nullptr }
            , m_propertyTypeId{ serialization::TypeId::InvalidId }
        {
            m_nodeCurrent.push(scene);
        }

        bool Null()
        {
            m_keyStack.pop();

            //OutputDebugStringA((identString() + engine::string("Null()\n")).data());
            return true;
        }

        bool Bool(bool b)
        {
            switch (m_propertyTypeId)
            {
                case serialization::TypeId::Bool:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<bool>(readJsonValue_bool(b));
                    else
                        m_componentCurrent->insertLoadedValue<bool>(m_propertyName, readJsonValue_bool(b));
                }
                /*case serialization::TypeId::ButtonPush:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<engine::ButtonPush>(readJsonValue_buttonPush(b));
                    else
                        m_componentCurrent->insertLoadedValue<engine::ButtonPush>(m_propertyName, readJsonValue_buttonPush(b));
                }*/
                case serialization::TypeId::ButtonToggle:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<engine::ButtonToggle>(readJsonValue_buttonToggle(b));
                    else
                        m_componentCurrent->insertLoadedValue<engine::ButtonToggle>(m_propertyName, readJsonValue_buttonToggle(b));
                }
                default:
                {
                    //ASSERT(false, "Not a boolean value");
                    break;
                }
            }
            m_keyStack.pop();

            engine::string res;
            res += "Bool(";
            res += b ? "true" : "false";
            res += ")\n";
            //OutputDebugStringA((identString() + res).data());
            return true;
        }

        bool Int(int i)
        {
            switch (m_propertyTypeId)
            {
                case serialization::TypeId::Char:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<char>(readJsonValue_char(i));
                    else
                        m_componentCurrent->insertLoadedValue<char>(m_propertyName, readJsonValue_char(i));
                    break;
                }
                case serialization::TypeId::Short:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<short>(readJsonValue_short(i));
                    else
                        m_componentCurrent->insertLoadedValue<short>(m_propertyName, readJsonValue_short(i));
                    break;
                }
                case serialization::TypeId::Int:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<int>(readJsonValue_int(i));
                    else
                        m_componentCurrent->insertLoadedValue<int>(m_propertyName, readJsonValue_int(i));
                    break;
                }
                default:
                {
                    ASSERT(false, "Not an integer value");
                    break;
                }
            }
            m_keyStack.pop();

            /*char buf[64]; memset(&buf[0], 0, 64);
            sprintf_s(buf, "Int(%i)\n", i);
            engine::string res(buf);
            OutputDebugStringA((identString() + res).data());*/
            return true;
        }
        
        bool Uint(unsigned u)
        {
            switch (m_propertyTypeId)
            {
                case serialization::TypeId::Char:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<char>(readJsonValue_char((int)u));
                    else
                        m_componentCurrent->insertLoadedValue<char>(m_propertyName, readJsonValue_char((int)u));
                    break;
                }
                case serialization::TypeId::Short:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<short>(readJsonValue_short((int)u));
                    else
                        m_componentCurrent->insertLoadedValue<short>(m_propertyName, readJsonValue_short((int)u));
                    break;
                }
                case serialization::TypeId::Int:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<int>(readJsonValue_int((int)u));
                    else
                        m_componentCurrent->insertLoadedValue<int>(m_propertyName, readJsonValue_int((int)u));
                    break;
                }
                case serialization::TypeId::UnsignedChar:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<unsigned char>(readJsonValue_unsignedchar(u));
                    else
                        m_componentCurrent->insertLoadedValue<unsigned char>(m_propertyName, readJsonValue_unsignedchar(u));
                    break;
                }
                case serialization::TypeId::UnsignedShort:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<unsigned short>(readJsonValue_unsignedshort(u));
                    else
                        m_componentCurrent->insertLoadedValue<unsigned short>(m_propertyName, readJsonValue_unsignedshort(u));
                    break;
                }
                case serialization::TypeId::UnsignedInt:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<unsigned int>(readJsonValue_unsignedint(u));
                    else
                        m_componentCurrent->insertLoadedValue<unsigned int>(m_propertyName, readJsonValue_unsignedint(u));
                    break;
                }
                default:
                {
                    ASSERT(false, "Not an unsigned integer value");
                    break;
                }
            }
            m_keyStack.pop();

            /*char buf[64]; memset(&buf[0], 0, 64);
            sprintf_s(buf, "Uint(%u)\n", u);
            engine::string res(buf);
            OutputDebugStringA((identString() + res).data());*/
            return true;
        }

        bool Int64(int64_t /*i*/)
        {
            m_keyStack.pop();

            /*char buf[64]; memset(&buf[0], 0, 64);
            sprintf_s(buf, "Int64(%lld)\n", i);
            engine::string res(buf);
            OutputDebugStringA((identString() + res).data());*/
            return true;
        }

        bool Uint64(uint64_t /*u*/)
        {
            m_keyStack.pop();

            /*char buf[64]; memset(&buf[0], 0, 64);
            sprintf_s(buf, "Uint64(%llu)\n", u);
            engine::string res(buf);
            OutputDebugStringA((identString() + res).data());*/
            return true;
        }

        bool Double(double d)
        {
            switch (m_propertyTypeId)
            {
                case serialization::TypeId::Float:
                {
                    if(m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<float>(readJsonValue_float(d));
                    else
                        m_componentCurrent->insertLoadedValue<float>(m_propertyName, readJsonValue_float(d));
                    break;
                }
                case serialization::TypeId::Double:
                {
                    if (m_componentCurrent->hasVariant(m_propertyName))
                        m_componentCurrent->variant(m_propertyName).value<double>(readJsonValue_double(d));
                    else
                        m_componentCurrent->insertLoadedValue<double>(m_propertyName, readJsonValue_double(d));
                    break;
                }
                default:
                {
                    ASSERT(false, "Not a double value");
                    break;
                }
            }
            m_keyStack.pop();

            /*char buf[64]; memset(&buf[0], 0, 64);
            sprintf_s(buf, "Double(%f)\n", d);
            engine::string res(buf);
            OutputDebugStringA((identString() + res).data());*/
            return true;
        }

		bool String(const char* str, SizeType length, bool /*copy*/);

        bool StartObject()
        {
            // we always have root node
            if(m_objectStack.size() == 0)
                m_objectStack.push(serialization::ObjectTypes::Node);
            else
            {
                if (m_keyStack.top() == serialization::KeyTypes::NodeList)
                {
                    auto newChild = engine::make_shared<SceneNode>();
                    m_nodeCurrent.top()->addChild(newChild);
                    m_nodeCurrent.push(newChild);
                    m_objectStack.push(serialization::ObjectTypes::Node);
                }
                else if (m_keyStack.top() == serialization::KeyTypes::ComponentList)
                {
                    m_objectStack.push(serialization::ObjectTypes::Component);
                }
                else if (m_keyStack.top() == serialization::KeyTypes::PropertyList)
                {
                    m_objectStack.push(serialization::ObjectTypes::Property);
                }
                else
                {
                    ASSERT(false);
                }
            }

            //OutputDebugStringA((identString() + engine::string("StartObject()\n")).data());
            ++ident;
            return true;
        }

        bool EndObject(SizeType /*memberCount*/)
        {
            //LOG_INFO("Scene serialization EndObject does not use memberCount");
            --ident;

            /*engine::string temp;
            temp += "EndObject(";
            temp += std::to_string(memberCount);
            temp += ")\n";
            OutputDebugStringA((identString() + temp).data());*/

            if (m_objectStack.top() == serialization::ObjectTypes::Node)
                m_nodeCurrent.pop();
            m_objectStack.pop();

            return true;
        }
        
        bool Key(const char* str, SizeType length, bool /*copy*/)
        {
            //LOG_INFO("Scene serialization does not implement key copy");
            engine::string stdStr(str, length);
            auto parts = tokenize(stdStr);
            serialization::KeyTypes keyType = static_cast<serialization::KeyTypes>(stringToNumber<int>(parts[0]));
            m_propertyTypeId = static_cast<serialization::TypeId>(stringToNumber<int>(parts[1]));
            m_keyStack.push(keyType);

            /*switch (keyType)
            {
                case serialization::NodeName:
                {
                    m_keyStack.push(serialization::KeyTypes::NodeName);
                    break;
                }
                case serialization::NodeList:
                {
                    m_stack.push(DeSerializeTask::NodeArray);
                    break;
                }
                case serialization::Property:
                {
                    int temp;
                    engine::stringstream ss(parts[1]);
                    ss >> temp;
                    m_propertyTypeId = static_cast<serialization::TypeId>(temp);
                    m_propertyName = parts[2];
                    m_stack.push(DeSerializeTask::Property);
                    break;
                }
                case serialization::PropertyList:
                {
                    m_stack.push(DeSerializeTask::PropertyArray);
                    break;
                }
                case serialization::Component:
                {
                    m_stack.push(DeSerializeTask::Component);
                    break;
                }
                case serialization::ComponentList:
                {
                    m_stack.push(DeSerializeTask::ComponentArray);
                    break;
                }
            }*/

            /*engine::string temp;
            temp += "Key(";
            temp += str;
            temp += ", ";
            temp += std::to_string(length);
            temp += ", ";
            temp += copy ? "true" : "false";
            temp += ")\n";
            OutputDebugStringA((identString() + temp).data());*/
            return true;
        }
        
        bool StartArray()
        {
            //OutputDebugStringA((identString() + engine::string("StartArray()\n")).data());
            ++ident;

            return true;
        }

        bool EndArray(SizeType /*elementCount*/)
        {
            //LOG_INFO("Scene serialization EndArray does not use elementCount");
            --ident;

            /*engine::string temp;
            temp += "EndArray(";
            temp += std::to_string(elementCount);
            temp += ")\n";
            OutputDebugStringA((identString() + temp).data());*/

            /*if (m_stack.top() == DeSerializeTask::ComponentArray)
            {
                m_currentComponent = nullptr;
            }*/
            m_keyStack.pop();
            
            return true;
        }
    private:
        engine::shared_ptr<SceneNode> m_scene;

        std::stack<engine::shared_ptr<SceneNode>>  m_nodeCurrent;
        engine::shared_ptr<EngineComponent>        m_componentCurrent;

        serialization::TypeId m_propertyTypeId;
        engine::string m_propertyName;

        std::stack<serialization::ObjectTypes> m_objectStack;
        std::stack<serialization::KeyTypes> m_keyStack;
    };
}
