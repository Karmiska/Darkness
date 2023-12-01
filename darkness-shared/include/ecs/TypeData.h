#pragma once

#include <cstdint>
#include <new>

namespace ecs
{
    class TypeDataBase
    {
    public:
        virtual ~TypeDataBase() {}
        virtual void* rawData() = 0;
        virtual void swap(uint64_t a, uint64_t b) noexcept = 0;
        virtual void copy(TypeDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t elements) noexcept = 0;
    };

    template<typename T>
    class TypeData : public TypeDataBase
    {
        const bool complex = !(std::is_standard_layout<T>().value && std::is_trivial<T>().value);
    public:
        TypeData(T* data, size_t elements)
            : m_data{ data }
            , m_elements{ elements }
        {
            if (complex)
                new (m_data) T[elements];
        }

        ~TypeData()
        {
            if (complex)
                for (size_t i = 0; i < m_elements; ++i)
                    m_data[i].~T();
        }

        T* data() { return m_data; }
        const T* data() const { return m_data; }

        size_t elements() const { return m_elements; }

        void swap(uint64_t a, uint64_t b) noexcept override
        {
            std::swap(m_data[a], m_data[b]);
        }

        void copy(TypeDataBase* src, uint64_t srcIndex, uint64_t dstIndex, size_t elements) noexcept override
        {
            TypeData<T>& srcCom = *static_cast<TypeData<T>*>(src);
            for (int i = 0; i < elements; ++i)
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
