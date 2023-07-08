#pragma once

#include "ecs/ComponentData.h"
#include "ecs/ComponentTypeStorage.h"
#include "ecs/Entity.h"
#include "tools/ToolsCommon.h"
#include "ecs/TypeSort.h"

#include <functional>
#include <execution>
#include <type_traits>

#define PARALLEL_FOR_LOOP

namespace ecs
{
    #define VectorizationSize 64

    class Ecs
    {
    public:
        Ecs()
            //: m_entities{}
            //, m_entityReserve{ 65536 }
            //, m_entityIndex{ 0 }
            : m_emptyEntity{this, createEntityId(0, 0, EmptyArcheTypeId) }
        {
            //m_entities.resize(m_entityReserve);
        }

        Entity createEntity()
        {
            return m_emptyEntity;
            /*if (m_entityIndex >= m_entityReserve)
            {
                m_entityReserve *= 2;
                m_entityReserve = roundUpToMultiple(m_entityReserve, VectorizationSize);
                m_entities.reserve(m_entityReserve);
            }

            auto id = EntityId{ entityIdFromEntityIndexAndArcheType(m_entityIndex, EmptyArcheTypeId) };
            if (m_entityIndex >= m_entities.size())
                m_entities.emplace_back(id);
            else
                m_entities[m_entityIndex] = id;

            ++m_entityIndex;

            return Entity(this, m_entities[m_entityIndex]);*/
        };

        Entity getEntity(EntityId id)
        {
            return Entity(this, id);
            //Entity entity(this, id);
            //ComponentArcheTypeId archeType = entity.archeType();
            //return 
        };

    private:

        template<typename T>
        uint64_t typeIndex()
        {
            return ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>();
        }

        //template<typename T>
        //void reserveComponentData()
        //{
        //    auto componentIndex = typeIndex<T>();
        //    if (componentIndex >= m_componentData.size())
        //    {
        //        m_componentData.resize(componentIndex + 1);
        //        m_componentData[componentIndex] = std::any(ComponentData<typename std::remove_reference<T>::type>());
        //        std::any_cast<ComponentData<typename std::remove_reference<T>::type>&>(m_componentData[componentIndex]).resize(m_entityIndex);
        //    }
        //}

        // These unpack component type id:s to a vector from template arguments
        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackSystems(std::vector<ComponentTypeId>& result) { }

        template<typename T, typename... Rest>
        void unpackSystems(std::vector<ComponentTypeId>& result)
        {
            result.emplace_back(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
            unpackSystems<Rest& ...>(result);
        }

        // returns a pointer to the beginning of types component data
        template<typename T>
        typename std::remove_reference<T>::type* componentDataPointer(ComponentTypeId componentTypeIndex)
        {
            //reserveComponentData<typename std::remove_reference<T>::type>();
            return std::any_cast<ComponentData<typename std::remove_reference<T>::type>&>(m_componentData[componentTypeIndex]).data();
        }

        // these return a tuple of component data pointers [EcsTransform* data, EcsRigidBody* ptr, etc...]
        template<typename T>
        std::tuple<typename std::remove_reference<T>::type*> packComponentPointers(Ecs& ecs)
        {
            return std::make_tuple<typename std::remove_reference<T>::type*>(ecs.componentDataPointer<typename std::remove_reference<T>::type>(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>()));
        }

        template <typename T, typename Arg, typename... Args>
        std::tuple<typename std::remove_reference<T>::type*, typename std::remove_reference<Arg>::type*, Args...> packComponentPointers(Ecs& ecs)
        {
            return std::tuple_cat(
                std::make_tuple<typename std::remove_reference<T>::type*>(ecs.componentDataPointer<typename std::remove_reference<T>::type>(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>())),
                packComponentPointers<Arg, Args...>(ecs));
        }

        // these call a query lambda with component references. the tuple contains the root pointers to component datas and we index to correct entity component with entityId
        template<typename Function, typename Tuple, size_t ... I>
        auto callQueryLambda(Function f, Tuple t, const EntityId& entityId, std::index_sequence<I ...>)
        {
            return f(*(std::get<I>(t) + entityId) ...);
        }

        template<typename Function, typename Tuple>
        auto callQueryLambda(Function f, Tuple t, const EntityId& entityId)
        {
            static constexpr auto size = std::tuple_size<Tuple>::value;
            return callQueryLambda(f, t, entityId, std::make_index_sequence<size>{});
        }

        template<typename Func, typename... Args>
        void queryInternal(Func func)
        {
            std::vector<uint64_t> typeIndexes;
            unpackSystems<Args...>(typeIndexes);
            auto tuples = packComponentPointers<Args...>(*this);

        #ifdef PARALLEL_FOR_LOOP
            std::for_each(
                std::execution::par_unseq,
                m_entities.begin(),
                m_entities.end(),
                [&](auto&& entity)
        #else
            for (auto& entity : m_entities)
        #endif
            {
                if (hasComponents(Entity(this, entity), typeIndexes))
                    callQueryLambda(func, tuples, entity);

        #ifdef PARALLEL_FOR_LOOP
            });
        #else
        };
        #endif
        };

        template <typename F, typename T>
        struct function_traits : public function_traits<F, decltype(&std::remove_reference_t<T>::operator())>
        {};

        template <typename F, typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<F, ReturnType(ClassType::*)(Args...) const>
        {
            static void call(Ecs& ecs, F func)
            {
                // ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>()
                //auto tmp = instantiate_t<SortedTypeContainer, sorted_list_t<list<Args...>>>;
                //auto archeTypeId = ComponentTypeStorage::archetypeId<Args...>();
                ecs.queryInternal<F, Args...>(func);
            };
        };

    private:
        friend class Entity;
        void addComponent(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityId(entity.entityId);
            auto entityIndex = entityIndexFromEntityId(entity.entityId);
            auto chunkIndex = chunkIndexFromEntityId(entity.entityId);
            
            // remove the entity from it's current chunk
            m_chunks[currentArcheType][chunkIndex].free(entityIndex);
            
            // now update the type set and get the new archetype
            ArcheType temporaryArcheType(currentArcheType);
            temporaryArcheType.typeSet().emplace(componentTypeId);
            ArcheType newArcheType(temporaryArcheType.typeSet());
            auto newArcheTypeId = newArcheType.id();

            // make sure we have space for that archetype
            if (newArcheTypeId >= m_chunks.size())
                m_chunks.resize(newArcheTypeId + 1);

            // find a chunk with free space
            //auto& chunkList = m_chunks[newArcheTypeId];
            //uint64_t chunkIndex = 0;
            //for(auto&& ch : chunkList)
            //{
            //    if (ch.available())
            //    {
            //        entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
            //        return;
            //    }
            //    ++chunkIndex;
            //}

            // or create new chunk if one wasn't available
            //chunkList.emplace_back(Chunk(newArcheTypeId));
            //entity.entityId = createEntityId(chunkList.back().allocate(), chunkList.size()-1, newArcheTypeId);
        }

        bool hasComponent(const Entity& entity, ComponentTypeId componentTypeId)
        {
            ArcheType currentArcheType(archeTypeIdFromEntityId(entity.entityId));
            return currentArcheType.typeSet().contains(componentTypeId);
        }

        void removeComponent(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityId(entity.entityId);
            auto entityIndex = entityIndexFromEntityId(entity.entityId);
            auto chunkIndex = chunkIndexFromEntityId(entity.entityId);

            // remove the entity from it's current chunk
            m_chunks[currentArcheType][chunkIndex].free(entityIndex);

            // now update the type set and get the new archetype
            ArcheType temporaryArcheType(currentArcheType);
            temporaryArcheType.typeSet().erase(componentTypeId);
            ArcheType newArcheType(temporaryArcheType.typeSet());
            auto newArcheTypeId = newArcheType.id();

            // make sure we have space for that archetype
            if (newArcheTypeId >= m_chunks.size())
                m_chunks.resize(newArcheTypeId + 1);

            // find a chunk with free space
            //auto& chunkList = m_chunks[newArcheTypeId];
            //uint64_t chunkIndex = 0;
            //for (auto&& ch : chunkList)
            //{
            //    if (ch.available())
            //    {
            //        entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
            //        return;
            //    }
            //    ++chunkIndex;
            //}
            //
            //// or create new chunk if one wasn't available
            //chunkList.emplace_back(Chunk(newArcheTypeId));
            //entity.entityId = createEntityId(chunkList.back().allocate(), chunkList.size() - 1, newArcheTypeId);
        }

        bool hasComponents(const Entity& entity, const std::vector<ComponentTypeId>& componentIds)
        {
            for (auto&& id : componentIds)
                if (!hasComponent(entity, id))
                    return false;
            return true;
        }

    public:
        template<typename F>
        void query(F func)
        {
            typedef function_traits<F, decltype(func)> traits;
            traits::call(*this, func);
        }

        template<typename Component>
        Component& query(EntityId entityId)
        {
            return *(componentDataPointer<Component>(typeIndex<Component>()) + entityId);
        }

        /*template<typename A, typename B>
        void refreshTypeAllocations()
        {
            auto systemsA = typeIndex<A>();
            auto systemsB = typeIndex<B>();

            if (systemsA == InvalidTypeId || systemsB == InvalidTypeId)
                return;

            reserveComponentData<A>();
            reserveComponentData<B>();

            auto skipListSize = m_entities.size();
            skipListSize = roundUpToMultiple(skipListSize, VectorizationSize) / VectorizationSize;
            m_entityIndexes.resize(skipListSize);
            for (int i = 0; i < m_entityIndexes.size(); ++i)
                m_entityIndexes[i] = &m_entities[i * VectorizationSize];
        }

        template<typename A, typename B, typename C, typename D>
        void refreshTypeAllocations()
        {
            auto systemsA = typeIndex<A>();
            auto systemsB = typeIndex<B>();
            auto systemsC = typeIndex<C>();
            auto systemsD = typeIndex<D>();

            if (systemsA == InvalidTypeId || systemsB == InvalidTypeId || systemsC == InvalidTypeId || systemsD == InvalidTypeId)
                return;

            reserveComponentData<A>();
            reserveComponentData<B>();
            reserveComponentData<C>();
            reserveComponentData<D>();

            auto skipListSize = m_entities.size();
            skipListSize = roundUpToMultiple(skipListSize, VectorizationSize) / VectorizationSize;
            m_entityIndexes.resize(skipListSize);
            for (int i = 0; i < m_entityIndexes.size(); ++i)
                m_entityIndexes[i] = &m_entities[i * VectorizationSize];
        }*/

        template<typename F>
        engine::vector<F>& system()
        {
            auto system = typeIndex<F>();
            return std::any_cast<ComponentData<typename std::remove_reference<F>::type>&>(m_componentData[system]).storage();
        }

    private:
        Entity m_emptyEntity;
        engine::vector<engine::vector<Chunk>> m_chunks;
        
        //engine::vector<EntityId*> m_entityIndexes;
        engine::vector<EntityId> m_entities;
        engine::vector<std::any> m_componentData;
        //size_t m_entityReserve;
        //size_t m_entityIndex;
};
}
