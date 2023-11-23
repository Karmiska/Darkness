#pragma once

#include "ecs/ComponentData.h"
#include "ecs/ComponentTypeStorage.h"
#include "ecs/ChunkStorage.h"
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

    class Entity;

    class Ecs
    {
    public:
        Ecs()
            : m_emptyEntity{ this, &m_componentTypeStorage, createEntityId(0, 0, InvalidArcheTypeId) }
            , m_componentTypeStorage{}
            , m_archeTypeStorage{ m_componentTypeStorage }
            , m_chunkStorage{ m_componentTypeStorage, m_archeTypeStorage }
        {
        }

        Entity createEntity()
        {
            return m_emptyEntity;
        };

        Entity getEntity(EntityId id)
        {
            return Entity(this, &m_componentTypeStorage, id);
        };

        ComponentTypeStorage& componentTypeStorage()
        {
            return m_componentTypeStorage;
        }

        ArcheTypeStorage& archeTypeStorage()
        {
            return m_archeTypeStorage;
        }

        void updateArcheTypeStorage(ComponentArcheTypeId id)
        {
            if (id >= m_chunks.size())
            {
                m_chunks.resize(id + 1);
                //m_lastUsedChunk.resize(id + 1, -1);
            }
        }

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
            result.set(m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>());
            unpackSystems<Rest& ...>(result);
        }

        // returns a pointer to the beginning of types component data
        template<typename T>
        std::tuple<typename std::remove_reference<T>::type*> packComponentPointers(Chunk& chunk)
        {
            return std::make_tuple<typename std::remove_reference<T>::type*>(chunk.componentDataPointer<typename std::remove_reference<T>::type>(m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>()));
        }

        template <typename T, typename Arg, typename... Args>
        std::tuple<typename std::remove_reference<T>::type*, typename std::remove_reference<Arg>::type*, Args...> packComponentPointers(Chunk& chunk)
        {
            return std::tuple_cat(
                std::make_tuple<typename std::remove_reference<T>::type*>(chunk.componentDataPointer<typename std::remove_reference<T>::type>(m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>())),
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
            engine::vector<ComponentArcheTypeId> archetypes = m_archeTypeStorage.archeTypesThatContain(typeIndexes);

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
                    if (!chunk->empty())
                    {
                        auto tuples = packComponentPointers<Args...>(*chunk);


                        for (auto&& entity : *chunk)
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
                ecs.queryInternal<F, Args...>(func);
            };
        };

    private:
        friend class Entity;

        void addComponent(Entity& entity, ComponentTypeId componentTypeId)
        {
            modifyComponent(entity, m_archeTypeStorage.archeType(archeTypeIdFromEntityId(entity.entityId), componentTypeId).id());
        }

        void addComponents(Entity& entity, const ArcheTypeSet& typeIndexes)
        {
            auto archeType = archeTypeIdFromEntityId(entity.entityId);
            if(archeType != InvalidArcheTypeId)
                modifyComponent(entity, m_archeTypeStorage.archeType(m_archeTypeStorage.archeType(archeType).typeSet() | typeIndexes).id());
            else
                modifyComponent(entity, m_archeTypeStorage.archeType(typeIndexes).id());
        }

        void addComponents(Entity& entity, const ArcheTypeSet& typeIndexes, ComponentArcheTypeId id)
        {
            auto archeType = archeTypeIdFromEntityId(entity.entityId);
            if (archeType != InvalidArcheTypeId)
                modifyComponent(entity, m_archeTypeStorage.archeType(m_archeTypeStorage.archeType(archeType).typeSet() | typeIndexes).id());
            else
                modifyComponent(entity, id);
        }

        void setComponents(Entity& entity, const ArcheTypeSet& typeIndexes)
        {
            setComponents(entity, m_archeTypeStorage.archeTypeIdFromSet(typeIndexes));
        }

        void setComponents(Entity& entity, ComponentArcheTypeId id)
        {
            modifyComponent(entity, id);
        }

        void removeComponent(Entity& entity, ComponentTypeId componentTypeId)
        {
            auto archeTypeSet = m_archeTypeStorage.archeType(archeTypeIdFromEntityId(entity.entityId)).typeSet();
            archeTypeSet.clear(componentTypeId);
            modifyComponent(entity, m_archeTypeStorage.archeType(archeTypeSet).id());
        }

        void modifyComponent(Entity& entity, ComponentArcheTypeId newArcheTypeId)
        {
            // Entity old info
            ComponentArcheTypeId oldArcheTypeId = archeTypeIdFromEntityId(entity.entityId);
            auto oldEntityIndex = entityIndexFromEntityId(entity.entityId);
            auto oldChunkIndex = chunkIndexFromEntityId(entity.entityId);

            // make sure we have space for that archetype
            updateArcheTypeStorage(newArcheTypeId);

            // allocate new entity
            entity.entityId = allocateNewEntity(newArcheTypeId);
            auto newEntityIndex = entityIndexFromEntityId(entity.entityId);
            auto newEntityChunkIndex = chunkIndexFromEntityId(entity.entityId);

            // if this entity was in some other archetype previously
            if (oldArcheTypeId != InvalidArcheTypeId)
            {
                // swap the removed entity to last in chunk to avoid holes
                auto& oldChunk = m_chunks[oldArcheTypeId][oldChunkIndex];
                auto chunkSize = oldChunk->size();
                if (oldEntityIndex < chunkSize - 1)
                {
                    oldChunk->swap(oldEntityIndex, chunkSize - 1);
                    oldEntityIndex = chunkSize - 1;
                }

                // copy over values from the previous types
                auto& newChunk = m_chunks[newArcheTypeId][newEntityChunkIndex];
                newChunk->copy(*oldChunk, oldEntityIndex, newEntityIndex, 1);

                // remove the entity from it's old chunk
                oldChunk->free(oldEntityIndex);

                // if the chunk is now empty. remove it.
                if (oldChunk->empty())
                {
                    //if (m_lastUsedChunk[oldArcheTypeId] == oldChunkIndex)
                    //    m_lastUsedChunk[oldArcheTypeId] = -1;
                    lastArcheTypeId = InvalidArcheTypeId;
                    m_lastUsedChunk = nullptr;
                    m_chunkStorage.freeChunk(oldArcheTypeId, m_chunks[oldArcheTypeId][oldChunkIndex]);
                    m_chunks[oldArcheTypeId].erase(m_chunks[oldArcheTypeId].begin() + oldChunkIndex);
                }
            }
        }

        

        EntityId allocateNewEntity(ComponentArcheTypeId archeTypeId)
        {
            // check if we can fit the new entity to last used chunk
            //auto& lastUsed = m_lastUsedChunk[archeTypeId];
            //if (lastUsed != -1)
            //{
            //    auto ch = m_chunks[archeTypeId][lastUsed];
            //    if (ch->available())
            //    {
            //        return createEntityId(ch->allocate(), lastUsed, archeTypeId);
            //    }
            //}
            if(lastArcheTypeId == archeTypeId && m_lastUsedChunk->available())
                return createEntityId(m_lastUsedChunk->allocate(), m_lastUsedChunkId, archeTypeId);

            // TODO: mark possible candidate chunks
            //       based on when chunks have had entities removed
            //       from them
            // find a chunk with free space
            auto& chunkList = m_chunks[archeTypeId];
            //for (int i = 0; i < chunkList.size(); ++i)
            //{
            //    auto ch = chunkList[i];
            //    if (ch->available())
            //    {
            //        m_lastUsedChunkId = i;
            //        m_lastUsedChunk = ch;
            //        lastArcheTypeId = archeTypeId;
            //        return createEntityId(ch->allocate(), i, archeTypeId);
            //    }
            //}

            // or create new chunk if one wasn't available
            chunkList.emplace_back(m_chunkStorage.allocateChunk(archeTypeId));
            //lastUsed = chunkList.size() - 1;
            m_lastUsedChunkId = chunkList.size() - 1;
            m_lastUsedChunk = chunkList[m_lastUsedChunkId];
            lastArcheTypeId = archeTypeId;
            return createEntityId(chunkList.back()->allocate(), chunkList.size() - 1, archeTypeId);
        }

        bool hasComponent(const Entity& entity, ComponentTypeId componentTypeId)
        {
            auto currentArcheType = m_archeTypeStorage.archeType(archeTypeIdFromEntityId(entity.entityId));
            return currentArcheType.contains(componentTypeId);
        }

        void* component(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityId(entity.entityId);
            auto chunkIndex = chunkIndexFromEntityId(entity.entityId);

            return m_chunks[currentArcheType][chunkIndex]->componentDataPointer(componentTypeId);
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
            std::vector<ComponentArcheTypeId> archetypes = m_archeTypeStorage.archeTypesThatContain(typeSet);

            auto dst = data;
            for (auto&& archetype : archetypes)
            {
                for (auto& chunk : m_chunks[archetype])
                {
                    if (!chunk->empty())
                    {
                        auto srcPtr = chunk->componentDataPointer<typename std::remove_reference<T>::type>(typeId);
                        memcpy(
                            dst, 
                            srcPtr, 
                            chunk->size() * ComponentTypeStorage::typeInfo(typeId).typeSizeBytes
                        );
                        dst += chunk->size();
                    }

                }
            }
        }

        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackTypes(ArcheTypeSet& result) { }

        template<typename T, typename... Rest>
        void unpackTypes(ArcheTypeSet& result)
        {
            result.set(m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>());
            unpackTypes<Rest& ...>(result);
        }

        template<typename T, typename... Rest>
        void prewarmArcheType(size_t bytes)
        {
            ArcheTypeSet typeSet;
            unpackTypes<T, Rest...>(typeSet);

            auto id = m_archeTypeStorage.archeTypeIdFromSet(typeSet);
            m_chunkStorage.reserve(id, bytes / PreferredChunkSizeBytes);
        }

    private:
        friend class Entity;
        Entity m_emptyEntity;
        engine::vector<engine::vector<Chunk*>> m_chunks;
        //engine::vector<int> m_lastUsedChunk;

        ComponentArcheTypeId lastArcheTypeId;
        Chunk* m_lastUsedChunk;
        int m_lastUsedChunkId;

        ComponentTypeStorage m_componentTypeStorage;
        ArcheTypeStorage m_archeTypeStorage;
        ChunkStorage m_chunkStorage;

        //engine::vector<EntityId> m_entities;
};
}
