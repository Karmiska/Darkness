#pragma once

#include "ecs/ComponentData.h"
#include "ecs/ComponentTypeStorage.h"
#include "ecs/ChunkStorage.h"
#include "ecs/Entity.h"
#include "tools/ToolsCommon.h"
#include "ecs/TypeSort.h"
#include "ecs/EcsShared.h"

#include <functional>
#include <execution>
#include <type_traits>

#define PARALLEL_FOR_LOOP
#undef LOOK_FOR_PARTIAL_CHUNKS

namespace ecs
{
    class Ecs
    {
    public:
        Ecs()
            : m_emptyEntity{ this, &m_componentTypeStorage, createEntityId(0, 0, InvalidArcheTypeId) }
            , m_componentTypeStorage{}
            , m_archeTypeStorage{ m_componentTypeStorage }
            , m_chunkStorage{ m_componentTypeStorage, m_archeTypeStorage }
            , m_lastArcheTypeId{ InvalidArcheTypeId }
            , m_archeTypeCount{ 0 }
        {
            updateArcheTypeStorage(MaximumArcheTypes);
        }

        Entity createEntity()
        {
            return m_emptyEntity;
        };

        Entity getEntity(EntityId id)
        {
            return Entity(this, &m_componentTypeStorage, id);
        };

        template<typename T, typename... Rest>
        ArcheType archeType()
        {
            ArcheTypeSet typeIndexes;
            unpackTypes<T, Rest...>(typeIndexes);
            return m_archeTypeStorage.archeType(typeIndexes);
        }

    private:
        using ChunkId = int;

        void updateArcheTypeStorage(ComponentArcheTypeId id, size_t reserveSpace = 0)
        {
            if (id >= m_archeTypeCount)
            {
                auto newSize = id + 1;
                m_chunks.resize(newSize);
                if (reserveSpace)
                    m_chunks[newSize - 1].reserve(reserveSpace);
#ifdef LOOK_FOR_PARTIAL_CHUNKS
                m_chunkMask.resize(newSize);
#endif
                m_lastUsedChunkPerArcheType.resize(newSize);
                m_partiallyFullChunkIds.resize(newSize);
                m_archeTypeCount = newSize;
            }
        }

        ComponentTypeStorage& componentTypeStorage()
        {
            return m_componentTypeStorage;
        }

        ArcheTypeStorage& archeTypeStorage()
        {
            return m_archeTypeStorage;
        }

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

                // if the chunk is now empty. remove it. (keeping at least one chunk ready even if it's empty)
                if (oldChunk->empty() && m_chunks[oldArcheTypeId].size() > 1)
                {
                    auto& lastUsedPerArcheType = m_lastUsedChunkPerArcheType[oldArcheTypeId];
                    if (lastUsedPerArcheType.id == oldChunkIndex)
                    {
                        lastUsedPerArcheType.id = -1;
                        lastUsedPerArcheType.chunk = nullptr;
                    }
                    if (m_lastArcheTypeId == oldArcheTypeId)
                    {
                        m_lastArcheTypeId = InvalidArcheTypeId;
                        m_lastUsedChunk.id = -1;
                        m_lastUsedChunk.chunk = nullptr;
                    }

                    for (auto partial = m_partiallyFullChunkIds[oldArcheTypeId].begin();
                        partial != m_partiallyFullChunkIds[oldArcheTypeId].end(); ++partial)
                    {
                        if (partial->id == oldChunkIndex)
                            partial = m_partiallyFullChunkIds[oldArcheTypeId].erase(partial);
                        else if (partial->id > oldChunkIndex)
                            ++partial->id;
                    }

                    m_chunkStorage.freeChunk(oldArcheTypeId, m_chunks[oldArcheTypeId][oldChunkIndex]);
                    m_chunks[oldArcheTypeId].erase(m_chunks[oldArcheTypeId].begin() + oldChunkIndex);
                }
                else if (!oldChunk->full())
                {
                    bool found = false;
                    for (auto&& lastUsed : m_partiallyFullChunkIds[oldArcheTypeId])
                    {
                        if (lastUsed.chunk == oldChunk)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        m_partiallyFullChunkIds[oldArcheTypeId].emplace_back(CachedLastChunk{ oldChunk, static_cast<int>(oldChunkIndex) });
                }
            }
        }

        EntityId allocateNewEntity(ComponentArcheTypeId archeTypeId)
        {
            // first we check if we're allocating from same archetype than last time
            // this is by far fastest as long as archetype doesn't change
            if (m_lastArcheTypeId == archeTypeId && m_lastUsedChunk.chunk->available())
                    return createEntityId(
                        m_lastUsedChunk.chunk->allocate(),
                        m_lastUsedChunk.id, archeTypeId);

            // then we check the last used chunk from the archetype specific last chunk
            if (m_lastArcheTypeId != archeTypeId && m_lastArcheTypeId != InvalidArcheTypeId)
            {
                auto& perArcheType = m_lastUsedChunkPerArcheType[archeTypeId];
                if (perArcheType.chunk && perArcheType.chunk->available())
                {
                    m_lastArcheTypeId = archeTypeId;
                    m_lastUsedChunk.id = perArcheType.id;
                    m_lastUsedChunk.chunk = perArcheType.chunk;
                    return createEntityId(
                        perArcheType.chunk->allocate(),
                        perArcheType.id, archeTypeId);
                }
            }

            // next we'll check if there are chunks that have gotten more space
            // as a result of erasing some entity from them
            for (auto notFull = m_partiallyFullChunkIds[archeTypeId].begin(); notFull != m_partiallyFullChunkIds[archeTypeId].end(); ++notFull)
            {
                if (notFull->chunk->available())
                {
                    m_lastArcheTypeId = archeTypeId;
                    m_lastUsedChunk.id = notFull->id;
                    m_lastUsedChunk.chunk = notFull->chunk;
                    m_partiallyFullChunkIds[archeTypeId].erase(notFull);
                    return createEntityId(
                        m_lastUsedChunk.chunk->allocate(),
                        m_lastUsedChunk.id, archeTypeId);
                }
                else
                    notFull = m_partiallyFullChunkIds[archeTypeId].erase(notFull);
            }

            auto& chunkList = m_chunks[archeTypeId];

#ifdef LOOK_FOR_PARTIAL_CHUNKS
            // this would find free space from the entire chunk list
            // but it can get really slow!
            auto& chunkMask = m_chunkMask[archeTypeId];
            auto nonFullChunk = chunkMask.begin();
            if (nonFullChunk != chunkMask.end())
            {
                auto emptyChunkId = *nonFullChunk;
                m_lastUsedChunk.id = emptyChunkId;
                
                // make sure we actually have the necessary amount of chunks
                while (emptyChunkId >= chunkList.size())
                    chunkList.emplace_back(m_chunkStorage.allocateChunk(archeTypeId));
                m_lastUsedChunk.chunk = chunkList[emptyChunkId];
                m_lastArcheTypeId = archeTypeId;
            
                if (m_lastUsedChunk.chunk->available())
                    return createEntityId(m_lastUsedChunk.chunk->allocate(), m_lastUsedChunk.id, archeTypeId);
                else
                {
                    chunkMask.clear(m_lastUsedChunk.id);
                }
            }
#endif

            // then we'll just create a new chunk
            chunkList.emplace_back(m_chunkStorage.allocateChunk(archeTypeId));

#ifdef LOOK_FOR_PARTIAL_CHUNKS
            // resize chunk mask
            auto newChunkCount = chunkList.size();
            if (newChunkCount > chunkMask.sizeBits())
            {
                if (chunkMask.sizeBits() == 0)
                {
                    chunkMask.resize(512, true);
                }
                else
                {
                    chunkMask.resize(chunkMask.sizeBits() * 2, true);
                }
            }
#endif

            m_lastUsedChunk.id = chunkList.size() - 1;
            m_lastUsedChunk.chunk = chunkList[m_lastUsedChunk.id];
            m_lastArcheTypeId = archeTypeId;
            m_lastUsedChunkPerArcheType[archeTypeId].chunk = m_lastUsedChunk.chunk;
            m_lastUsedChunkPerArcheType[archeTypeId].id = m_lastUsedChunk.id;
            return createEntityId(m_lastUsedChunk.chunk->allocate(), m_lastUsedChunk.id, archeTypeId);
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
            auto typeId = m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>();
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
                            chunk->size() * m_componentTypeStorage.typeInfo(typeId).typeSizeBytes
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

        // Chunk storage can be "preheated" to have chunks of certain archetype ready
        // so we can do memory allocation beforehand.
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
        size_t m_archeTypeCount;
        engine::vector<engine::vector<Chunk*>> m_chunks;
#ifdef LOOK_FOR_PARTIAL_CHUNKS
        engine::vector<engine::BitSetDynamic> m_chunkMask;
#endif
        
        // for fast path check if we're modifying the same archetype as last time
        struct CachedLastChunk
        {
            Chunk* chunk = nullptr;
            ChunkId id = -1;
        };
        ComponentArcheTypeId m_lastArcheTypeId;
        CachedLastChunk m_lastUsedChunk;

        // somewhat fast path if the archetype has changed
        engine::vector<CachedLastChunk> m_lastUsedChunkPerArcheType;

        // list of partly full chunks
        engine::vector<engine::vector<CachedLastChunk>> m_partiallyFullChunkIds;

        ComponentTypeStorage m_componentTypeStorage;
        ArcheTypeStorage m_archeTypeStorage;
        ChunkStorage m_chunkStorage;

        //engine::vector<EntityId> m_entities;
};
}
