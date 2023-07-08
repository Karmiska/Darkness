#pragma once

#include "containers/vector.h"
#include "ComponentTypeStorage.h"
#include "Entity.h"
#include <stack>

namespace ecs
{
    constexpr size_t PreferredChunkSizeBytes = 16 * 1024;

    class Chunk
    {
    public:
        Chunk(ComponentArcheTypeId archeType)
            : m_used{ 0 }
        {
            auto typeIds = ArcheTypeStorage::instance().typeSetFromArcheTypeId(archeType);
            uint32_t combinedTypeBytes = 0;
            for (auto&& type : typeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                combinedTypeBytes += typeInfo.typeSizeBytes;
            }
            combinedTypeBytes += sizeof(Entity);
            
            m_elements = PreferredChunkSizeBytes / combinedTypeBytes;
            
            for (auto&& type : typeIds)
            {
                auto typeInfo = ComponentTypeStorage::typeInfo(type);
                m_componentData.emplace_back(typeInfo.create(m_elements));
            }

            for (int i = 0; i < m_elements; ++i)
                m_free.push(i);
        }

        size_t size() const
        {
            return m_used;
        }

        size_t capacity() const
        {
            return m_elements;
        }

        size_t available() const
        {
            return capacity() - size();
        }

        EntityId allocate()
        {
            auto id = m_free.top();
            m_free.pop();
            ++m_used;
            return id;
        }

        void free(EntityId id)
        {
            m_free.push(entityIndexFromEntityId(id));
            --m_used;
        }
    private:
        size_t m_elements;
        engine::vector<ComponentDataBase*> m_componentData;
        std::stack<uint16_t> m_free;
        size_t m_used;
    };
}
