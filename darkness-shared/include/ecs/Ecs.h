#pragma once

#include "ecs/Chunk.h"
#include "ecs/TypeStorage.h"
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
    private:
        template<typename... T>
        typename std::enable_if<sizeof...(T) == 0>::type
            unpackTypes(ArcheTypeSet& result) { }

        template<typename T, typename... Rest>
        void unpackTypes(ArcheTypeSet& result)
        {
            result.set(m_componentTypeStorage.typeId<typename std::remove_reference<T>::type>());
            unpackTypes<Rest& ...>(result);
        }
    public:
        Ecs()
            : m_componentTypeStorage{}
            , m_archeTypeStorage{ m_componentTypeStorage }
            , m_chunkStorage{ m_componentTypeStorage, m_archeTypeStorage }
            , m_lastArcheTypeId{ InvalidArcheTypeId }
            , m_archeTypeCount{ 0 }
        {
            updateArcheTypeStorage(MaximumEcsArcheTypes);
        }

        Entity createEntity()
        {
            auto addr = createEntityAddress(0, 0, InvalidArcheTypeId);
            m_entities.emplace_back(addr);
            return Entity{ this, &m_componentTypeStorage, m_entities.size()-1, addr };
        };

        Entity getEntity(EntityId id)
        {
            return Entity(this, &m_componentTypeStorage, id, m_entities[id]);
        };

        void destroyEntity(EntityId id)
        {
            // swap places with last entityId
            auto aId = id;
            auto bId = m_entities.size() - 1;
            if (aId != bId)
            {
                // update the last entity to point to new index
                {
                    auto address = m_entities[bId];
                    ComponentArcheTypeId archeTypeId = archeTypeIdFromEntityAddress(address);
                    auto entityIndex = entityIndexFromEntityAddress(address);
                    auto chunkIndex = chunkIndexFromEntityAddress(address);
                    m_chunks[archeTypeId][chunkIndex]->entities()[entityIndex] = aId;
                }

                //ongelma on se että chunkkeja ei voi poistaa koska entity id:ssä on chunkki indexi.
                //poista se entity id:stä ja lisää se entity listaan addressin lisäksi

                // update the removed entity to point to last index
                {
                    auto address = m_entities[aId];
                    ComponentArcheTypeId archeTypeId = archeTypeIdFromEntityAddress(address);
                    auto entityIndex = entityIndexFromEntityAddress(address);
                    auto chunkIndex = chunkIndexFromEntityAddress(address);
                    m_chunks[archeTypeId][chunkIndex]->entities()[entityIndex] = bId;
                }

                // swap the addressing
                std::swap(m_entities[aId], m_entities[bId]);

                id = bId;
            }

            // remove the entity from a chunk
            {
                auto address = m_entities[id];
                ComponentArcheTypeId archeTypeId = archeTypeIdFromEntityAddress(address);
                auto entityIndex = entityIndexFromEntityAddress(address);
                auto chunkIndex = chunkIndexFromEntityAddress(address);

                // swap the removed entity to last in chunk to avoid holes
                auto& chunk = m_chunks[archeTypeId][chunkIndex];
                auto chunkSize = chunk->size();

                // if not already last
                if (entityIndex < chunkSize - 1)
                {
                    chunk->swap(entityIndex, chunkSize - 1);

                    // update ecs -> chunk entity address for the entity that got moved from last position
                    m_entities[chunk->entities()[entityIndex]] = createEntityAddress(
                        entityIndex, chunkIndex, archeTypeId);

                    entityIndex = chunkSize - 1;
                }

                // remove the entity from it's old chunk
                chunk->free(entityIndex);

                possiblyFreeChunk(archeTypeId, chunkIndex, chunk);
            }
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

        TypeStorage& componentTypeStorage()
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
            return TypeStorage::typeId<typename std::remove_reference<T>::type>();
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
        auto callQueryLambda(Function f, Tuple t, const EntityAddress& entityAddress, std::index_sequence<I ...>)
        {
            return f(*(std::get<I>(t) + entityAddress) ...);
        }

        template<typename Function, typename Tuple>
        auto callQueryLambda(Function f, Tuple t, const EntityAddress& entityAddress)
        {
            static constexpr auto size = std::tuple_size<Tuple>::value;
            return callQueryLambda(f, t, entityAddress, std::make_index_sequence<size>{});
        }

        template<typename Func, typename... Args>
        void queryInternal(Func func)
        {
            // Component type id list
            ArcheTypeSet typeIndexes;
            unpackTypes<Args...>(typeIndexes);

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
            modifyComponent(entity, m_archeTypeStorage.archeType(archeTypeIdFromEntityAddress(entity.entityAddress), componentTypeId).id());
        }

        void addComponents(Entity& entity, const ArcheTypeSet& typeIndexes)
        {
            auto archeType = archeTypeIdFromEntityAddress(entity.entityAddress);
            if(archeType != InvalidArcheTypeId)
                modifyComponent(entity, m_archeTypeStorage.archeType(m_archeTypeStorage.archeType(archeType).typeSet() | typeIndexes).id());
            else
                modifyComponent(entity, m_archeTypeStorage.archeType(typeIndexes).id());
        }

        void addComponents(Entity& entity, const ArcheTypeSet& typeIndexes, ComponentArcheTypeId id)
        {
            auto archeType = archeTypeIdFromEntityAddress(entity.entityAddress);
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
            auto archeTypeSet = m_archeTypeStorage.archeType(archeTypeIdFromEntityAddress(entity.entityAddress)).typeSet();
            archeTypeSet.clear(componentTypeId);
            modifyComponent(entity, m_archeTypeStorage.archeType(archeTypeSet).id());
        }

        void possiblyFreeChunk(ComponentArcheTypeId archeTypeId, uint64_t chunkIndex, Chunk* chunk)
        {
            // if the chunk is now empty. remove it. (keeping at least one chunk ready even if it's empty)
            if (chunk->empty() && m_chunks[archeTypeId].size() > 1)
            {
                auto& lastUsedPerArcheType = m_lastUsedChunkPerArcheType[archeTypeId];
                if (lastUsedPerArcheType.id == chunkIndex)
                {
                    lastUsedPerArcheType.id = -1;
                    lastUsedPerArcheType.chunk = nullptr;
                }
                if (m_lastArcheTypeId == archeTypeId && m_lastUsedChunk.id == chunkIndex)
                {
                    m_lastArcheTypeId = InvalidArcheTypeId;
                    m_lastUsedChunk.id = -1;
                    m_lastUsedChunk.chunk = nullptr;
                }

                for (auto partial = m_partiallyFullChunkIds[archeTypeId].begin();
                    partial != m_partiallyFullChunkIds[archeTypeId].end();)
                {
                    if (partial->id == chunkIndex)
                        partial = m_partiallyFullChunkIds[archeTypeId].erase(partial);
                    else
                    {
                        if (partial->id > chunkIndex)
                            partial->id += 1;
                        ++partial;
                    }
                }

                m_chunkStorage.freeChunk(archeTypeId, m_chunks[archeTypeId][chunkIndex]);
                m_chunks[archeTypeId].erase(m_chunks[archeTypeId].begin() + chunkIndex);
            }
            else if (!chunk->full())
            {
                bool found = false;
                for (auto&& lastUsed : m_partiallyFullChunkIds[archeTypeId])
                {
                    if (lastUsed.chunk == chunk)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    m_partiallyFullChunkIds[archeTypeId].emplace_back(CachedLastChunk{ chunk, static_cast<int>(chunkIndex) });
            }
        }

        void modifyComponent(Entity& entity, ComponentArcheTypeId newArcheTypeId)
        {
            // Entity old info
            ComponentArcheTypeId oldArcheTypeId = archeTypeIdFromEntityAddress(entity.entityAddress);
            auto oldEntityIndex = entityIndexFromEntityAddress(entity.entityAddress);
            auto oldChunkIndex = chunkIndexFromEntityAddress(entity.entityAddress);

            // allocate new entity
            entity.entityAddress = allocateNewEntity(newArcheTypeId);
            
            // update ecs -> chunk binding
            m_entities[entity.entityId] = entity.entityAddress;

            auto newEntityIndex = entityIndexFromEntityAddress(entity.entityAddress);
            auto newEntityChunkIndex = chunkIndexFromEntityAddress(entity.entityAddress);

            // update chunk -> ecs binding
            m_chunks[newArcheTypeId][newEntityChunkIndex]->entities()[newEntityIndex] = entity.entityId;

            // if this entity was in some other archetype previously
            if (oldArcheTypeId != InvalidArcheTypeId)
            {
                // swap the removed entity to last in chunk to avoid holes
                auto& oldChunk = m_chunks[oldArcheTypeId][oldChunkIndex];
                auto chunkSize = oldChunk->size();

                // if not already last
                if (oldEntityIndex < chunkSize - 1)
                {
                    oldChunk->swap(oldEntityIndex, chunkSize - 1);

                    // update ecs -> chunk entity address for the entity that got moved from last position
                    m_entities[oldChunk->entities()[oldEntityIndex]] = createEntityAddress(
                        oldEntityIndex, oldChunkIndex, oldArcheTypeId);

                    oldEntityIndex = chunkSize - 1;
                }

                // copy over values from the previous types
                auto& newChunk = m_chunks[newArcheTypeId][newEntityChunkIndex];
                newChunk->copy(*oldChunk, oldEntityIndex, newEntityIndex, 1);

                // remove the entity from it's old chunk
                oldChunk->free(oldEntityIndex);

                possiblyFreeChunk(oldArcheTypeId, oldChunkIndex, oldChunk);
            }
        }

        EntityAddress allocateNewEntity(ComponentArcheTypeId archeTypeId)
        {
            // first we check if we're allocating from same archetype than last time
            // this is by far fastest as long as archetype doesn't change
            if (m_lastArcheTypeId == archeTypeId && m_lastUsedChunk.chunk->available())
                    return createEntityAddress(
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
                    return createEntityAddress(
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
                    return createEntityAddress(
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
            return createEntityAddress(m_lastUsedChunk.chunk->allocate(), m_lastUsedChunk.id, archeTypeId);
        }

        bool hasComponent(const Entity& entity, ComponentTypeId componentTypeId)
        {
            auto currentArcheType = m_archeTypeStorage.archeType(archeTypeIdFromEntityAddress(entity.entityAddress));
            return currentArcheType.contains(componentTypeId);
        }

        void* component(Entity& entity, ComponentTypeId componentTypeId)
        {
            // find out the archetype
            ComponentArcheTypeId currentArcheType = archeTypeIdFromEntityAddress(entity.entityAddress);
            auto chunkIndex = chunkIndexFromEntityAddress(entity.entityAddress);

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
        Component& query(EntityAddress entityAddress)
        {
            return *(componentDataPointer<Component>(typeIndex<Component>()) + entityAddress);
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

#if 0
        

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
#endif
    private:
        friend class Entity;
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

        TypeStorage m_componentTypeStorage;
        ArcheTypeStorage m_archeTypeStorage;
        ChunkStorage m_chunkStorage;

        // two way connection between an EntityId and the entity data inside a chunk.
        // m_entities[EntityId] -> EntityAddress -> Chunk -> m_entityIds[EntityAddress.index] -> m_entities[EntityId]
        engine::vector<EntityAddress> m_entities;
};
}
