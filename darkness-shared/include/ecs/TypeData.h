#pragma once

#include <cstdint>
#include <new>
#include <memory>
#include "tools/Debug.h"

namespace ecs
{
    class TypeDataBase
    {
    public:
        virtual ~TypeDataBase() {}
        virtual void* rawData() = 0;
        virtual size_t elements() const = 0;
        virtual void swap(uint64_t a, uint64_t b) noexcept = 0;
        virtual void copy(TypeDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t elements) noexcept = 0;
    };

    template<typename T>
    class TypeData : public TypeDataBase
    {
    public:
        TypeData()
            : m_data{ nullptr }
            , m_elements{ 0 }
        {}

        TypeData(T* data, size_t elements)
            : m_data{ data }
            , m_elements{ elements }
        {
            std::uninitialized_default_construct_n(m_data, elements);
        }

        TypeData(const TypeData&) = delete;
        TypeData& operator=(const TypeData&) = delete;

        TypeData(TypeData&& data) noexcept
        {
            std::swap(m_data, data.m_data);
            std::swap(m_elements, data.m_elements);
        }

        ~TypeData()
        {
            if(m_data)
                std::destroy_n(m_data, m_elements);
        }

        T* data() { return m_data; }
        const T* data() const { return m_data; }

        size_t elements() const override { return m_elements; }

        void swap(uint64_t a, uint64_t b) noexcept override
        {
            ASSERT(a < elements());
            ASSERT(b < elements());
            std::swap(m_data[a], m_data[b]);
        }

        void copy(TypeDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t _elements) noexcept override
        {
            ASSERT(src != nullptr);
            ASSERT(srcIndex < src->elements());
            ASSERT(srcIndex + _elements <= src->elements());
            ASSERT(dstIndex < elements());
            ASSERT(dstIndex + _elements <= elements());

            TypeData<T>& srcCom = *static_cast<TypeData<T>*>(src);
            for (int i = 0; i < _elements; ++i)
            {
                m_data[dstIndex + i] = srcCom.m_data[srcIndex + i];
            }
        }

    private:
        void* rawData() override { return m_data; }

        T* m_data;
        size_t m_elements;
    };
}
