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
            : m_emptyEntity{this, createEntityId(0, 0, InvalidArcheTypeId) }
        {
        }

        Entity createEntity()
        {
            return m_emptyEntity;
        };

        Entity getEntity(EntityId id)
        {
            return Entity(this, id);
        };

    private:

        template<typename T>
        uint64_t typeIndex()
        {
            return ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>();
        }

        // These unpack component type id:s to a vector from template arguments
        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackSystems(ArcheTypeSet& result) { }

        template<typename T, typename... Rest>
        void unpackSystems(ArcheTypeSet& result)
        {
            result.set(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>());
            unpackSystems<Rest& ...>(result);
        }

        // returns a pointer to the beginning of types component data
        template<typename T>
        std::tuple<typename std::remove_reference<T>::type*> packComponentPointers(Chunk& chunk)
        {
            return std::make_tuple<typename std::remove_reference<T>::type*>(chunk.componentDataPointer<typename std::remove_reference<T>::type>(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>()));
        }

        template <typename T, typename Arg, typename... Args>
        std::tuple<typename std::remove_reference<T>::type*, typename std::remove_reference<Arg>::type*, Args...> packComponentPointers(Chunk& chunk)
        {
            return std::tuple_cat(
                std::make_tuple<typename std::remove_reference<T>::type*>(chunk.componentDataPointer<typename std::remove_reference<T>::type>(ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>())),
                packComponentPointers<Arg, Args...>(chunk));
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
            // Component type id list
            ArcheTypeSet typeIndexes;
            unpackSystems<Args...>(typeIndexes);

            // get archetypes that have all of these component types
            engine::vector<ComponentArcheTypeId> archetypes = ArcheTypeStorage::instance().archeTypesThatContain(typeIndexes);

            for (auto&& archetype : archetypes)
            {
#ifdef PARALLEL_FOR_LOOP
                std::for_each(
                    std::execution::par_unseq,
                    m_chunks[archetype].begin(),
                    m_chunks[archetype].end(),
                    [&](auto& chunk)
#else
                for (auto& chunk : m_chunks[archetype])
#endif
                {
                    if (!chunk.empty())
                    {
                        auto tuples = packComponentPointers<Args...>(chunk);


                        for (auto&& entity : chunk)
                        {
                            callQueryLambda(func, tuples, entity);
                        };
                    }

#ifdef PARALLEL_FOR_LOOP
                });
#else
                }
#endif
            }
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
            
            //TODO: need to copy over values!
            //TODO: need to move around entities to avoid holes

            // remove the entity from it's current chunk
            if(entity != m_emptyEntity && currentArcheType != InvalidArcheTypeId)
                m_chunks[currentArcheType][chunkIndex].free(entityIndex);
            
            // now update the type set and get the new archetype
            ArcheType temporaryArcheType(currentArcheType, componentTypeId);
            auto newArcheTypeId = temporaryArcheType.id();

            // make sure we have space for that archetype
            if (newArcheTypeId >= m_chunks.size())
            {
                m_chunks.resize(newArcheTypeId + 1);
                m_lastUsedChunk.resize(newArcheTypeId + 1, -1);
            }

            // find a chunk with free space
            auto lastUsed = m_lastUsedChunk[newArcheTypeId];
            if (lastUsed != -1)
            {
                auto& ch = m_chunks[newArcheTypeId][lastUsed];
                if (ch.available())
                {
                    entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
                    return;
                }
            }

            auto& chunkList = m_chunks[newArcheTypeId];
            chunkIndex = 0;
            for(int i = 0; i < chunkList.size(); ++i)
            {
                auto& ch = chunkList[i];
                if (ch.available())
                {
                    entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
                    m_lastUsedChunk[newArcheTypeId] = i;
                    return;
                }
                ++chunkIndex;
            }

            // or create new chunk if one wasn't available
            chunkList.emplace_back(Chunk(newArcheTypeId));
            entity.entityId = createEntityId(chunkList.back().allocate(), chunkList.size()-1, newArcheTypeId);
            m_lastUsedChunk[newArcheTypeId] = chunkList.size() - 1;
        }

        bool hasComponent(const Entity& entity, ComponentTypeId componentTypeId)
        {
            ArcheType currentArcheType(archeTypeIdFromEntityId(entity.entityId));
            return currentArcheType.contains(componentTypeId);
        }

        void removeComponent(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityId(entity.entityId);
            auto entityIndex = entityIndexFromEntityId(entity.entityId);
            auto chunkIndex = chunkIndexFromEntityId(entity.entityId);

            // TODO: need to copy over values!
            // TODO: need to move around entities to avoid holes

            // remove the entity from it's current chunk
            m_chunks[currentArcheType][chunkIndex].free(entityIndex);

            // now update the type set and get the new archetype
            auto archeTypeSet = ArcheType(currentArcheType).typeSet();
            archeTypeSet.set(componentTypeId, false);
            ArcheType newArcheType(archeTypeSet);
            auto newArcheTypeId = newArcheType.id();

            // make sure we have space for that archetype
            if (newArcheTypeId >= m_chunks.size())
            {
                m_chunks.resize(newArcheTypeId + 1);
                m_lastUsedChunk.resize(newArcheTypeId + 1, -1);
            }

            // find a chunk with free space
            auto lastUsed = m_lastUsedChunk[newArcheTypeId];
            if (lastUsed != -1)
            {
                auto& ch = m_chunks[newArcheTypeId][lastUsed];
                if (ch.available())
                {
                    entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
                    return;
                }
            }

            auto& chunkList = m_chunks[newArcheTypeId];
            chunkIndex = 0;
            for (int i = 0; i < chunkList.size(); ++i)
            {
                auto& ch = chunkList[i];
                if (ch.available())
                {
                    entity.entityId = createEntityId(ch.allocate(), chunkIndex, newArcheTypeId);
                    m_lastUsedChunk[newArcheTypeId] = i;
                    return;
                }
                ++chunkIndex;
            }
        }

        void* component(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityId(entity.entityId);
            auto entityIndex = entityIndexFromEntityId(entity.entityId);
            auto chunkIndex = chunkIndexFromEntityId(entity.entityId);

            return m_chunks[currentArcheType][chunkIndex].componentDataPointer(componentTypeId);
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

        template<typename T>
        void copyRaw(T* data, size_t elements)
        {
            // Component type id list
            auto typeId = ComponentTypeStorage::typeId<typename std::remove_reference<T>::type>();
            ArcheTypeSet typeSet;
            typeSet.set(typeId);

            // get archetypes that have all of these component types
            std::vector<ComponentArcheTypeId> archetypes = ArcheTypeStorage::instance().archeTypesThatContain(typeSet);

            auto dst = data;
            for (auto&& archetype : archetypes)
            {
                for (auto& chunk : m_chunks[archetype])
                {
                    if (!chunk.empty())
                    {
                        auto srcPtr = chunk.componentDataPointer<typename std::remove_reference<T>::type>(typeId);
                        memcpy(
                            dst, 
                            srcPtr, 
                            chunk.size() * ComponentTypeStorage::typeInfo(typeId).typeSizeBytes
                        );
                        dst += chunk.size();
                    }

                }
            }
        }

    private:
        Entity m_emptyEntity;
        engine::vector<engine::vector<Chunk>> m_chunks;
        engine::vector<int> m_lastUsedChunk;
        
        //engine::vector<EntityId> m_entities;
};
}
