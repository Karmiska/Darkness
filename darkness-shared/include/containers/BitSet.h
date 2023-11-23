#pragma once
#include <cstdint>
#include <iterator>

namespace engine
{
    inline uint8_t mask8(uint8_t index)
    {
        return ((uint16_t)0x00ff << index) & 0xff;
    }

    inline uint16_t mask16(uint16_t index)
    {
        return ((uint32_t)0x0000ffff << index) & 0xffff;
    }

    inline uint32_t mask32(uint32_t index)
    {
        return ((uint64_t)0x00000000ffffffff << index) & 0xffffffff;
    }

    inline uint64_t mask64(uint64_t index)
    {
        return index >= 64ull ? 0ull : 0xffffffffffffffffull << index;
    }

    class BitSetDynamic
    {
    public:
        BitSetDynamic()
            : m_data{ nullptr }
            , m_dataCount{ 0 }
            , m_size{ 0u }
        {}

        BitSetDynamic(size_t bitSize)
            : m_data{ static_cast<uint64_t*>(_aligned_malloc(roundUpToMultiple(bitSize, 64), 64)) }
            , m_dataCount{ roundUpToMultiple(bitSize, 64) / 64 }
            , m_size{ bitSize }
        {
            memset(m_data, 0, sizeof(uint64_t) * m_dataCount);
        }

        BitSetDynamic(const BitSetDynamic& bitset)
        {
            m_data = static_cast<uint64_t*>(_aligned_malloc(roundUpToMultiple(bitset.m_size, 64), 64));
            m_dataCount = bitset.m_dataCount;
            m_size = bitset.m_size;
            memcpy(m_data, bitset.m_data, m_dataCount * 64);
        }

        BitSetDynamic& operator=(const BitSetDynamic& bitset)
        {
            m_data = static_cast<uint64_t*>(_aligned_malloc(roundUpToMultiple(bitset.m_size, 64), 64));
            m_dataCount = bitset.m_dataCount;
            m_size = bitset.m_size;
            memcpy(m_data, bitset.m_data, m_dataCount * 64);
            return *this;
        }

        BitSetDynamic(BitSetDynamic&& bitset)
            : m_data{ nullptr }
            , m_dataCount{ 0 }
            , m_size{ 0 }
        {
            std::swap(m_data, bitset.m_data);
            std::swap(m_dataCount, bitset.m_dataCount);
            std::swap(m_size, bitset.m_size);
        }

        BitSetDynamic& operator=(BitSetDynamic&& bitset)
        {
            std::swap(m_data, bitset.m_data);
            std::swap(m_dataCount, bitset.m_dataCount);
            std::swap(m_size, bitset.m_size);
            return *this;
        }

        ~BitSetDynamic()
        {
            if (m_data)
            {
                _aligned_free(m_data);
                m_data = nullptr;
            }
        }

        void set(int index)
        {
            ASSERT(index >= 0 && index < m_size, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", m_size, index, m_size - 1);
            m_data[index / 64] |= ((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < m_size, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", m_size, index, m_size - 1);
            m_data[index / 64] &= ~((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < m_size, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", m_size, index, m_size - 1);
            return m_data[index / 64] & ((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        bool operator==(const BitSetDynamic& set) const
        {
            for (int i = 0; i < m_dataCount; ++i)
                if (m_data[i] != set.m_data[i])
                    return false;
            return true;
        }

        bool operator!=(const BitSetDynamic& set) const
        {
            return !(*this == set);
        }

        BitSetDynamic& operator&=(const BitSetDynamic& other) noexcept
        {
            for (int i = 0; i < m_dataCount; ++i)
                m_data[i] &= other.m_data[i];

            return *this;
        }
        BitSetDynamic& operator|=(const BitSetDynamic& other) noexcept
        {
            for (int i = 0; i < m_dataCount; ++i)
                m_data[i] |= other.m_data[i];

            return *this;
        }
        BitSetDynamic& operator^=(const BitSetDynamic& other) noexcept
        {
            for (int i = 0; i < m_dataCount; ++i)
                m_data[i] ^= other.m_data[i];

            return *this;
        }

        BitSetDynamic operator&(const BitSetDynamic& other) const noexcept
        {
            BitSetDynamic res = *this;

            for (int i = 0; i < m_dataCount; ++i)
                res.m_data[i] &= other.m_data[i];

            return res;
        }
        BitSetDynamic operator|(const BitSetDynamic& other) const noexcept
        {
            BitSetDynamic res = *this;

            for (int i = 0; i < m_dataCount; ++i)
                res.m_data[i] |= other.m_data[i];

            return res;
        }
        BitSetDynamic operator^(const BitSetDynamic& other) const noexcept
        {
            BitSetDynamic res = *this;

            for (int i = 0; i < m_dataCount; ++i)
                res.m_data[i] ^= other.m_data[i];

            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint64_t,                   // value_type
            uint64_t,                   // difference_type
            const uint64_t*,            // pointer
            uint64_t>                    // reference
        {
            Iterator(const BitSetDynamic* set, uint64_t index)
                : m_set{ set }
                , m_block{ 0u }
                , m_index{ index }
                , m_indexInternal{ static_cast<unsigned long>(index) }
            {
                if (index == m_set->m_size+1)
                    return;
                this->operator++();
            }

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                while (m_block < m_set->m_dataCount)
                {
                    unsigned long long mask = m_set->m_data[m_block];
                    mask &= mask64(m_indexInternal);

                    if (_BitScanForward64(&m_indexInternal, mask))
                    {
                        m_index = (m_block * 64) + m_indexInternal;
                        ++m_indexInternal;
                        return *this;
                    }
                    ++m_block;
                }
                m_index = m_set->m_size+1;
                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const BitSetDynamic* m_set;
            uint64_t m_block;
            uint64_t m_index;
            unsigned long m_indexInternal;
        };

        Iterator begin() const
        {
            return Iterator(this, 0);
        }
        Iterator end() const
        {
            return Iterator(this, m_size+1);
        }
    private:
        uint64_t* m_data;
        size_t m_dataCount;
        size_t m_size;
    };

    template<auto N>
    class BitSet
    {
        static_assert(N % 8 == 0, "BitSet supports only size multiples of 8");
        static constexpr size_t BitSetDataCount = (size_t)(((float)N / 64.0f) + 0.5f);
        static constexpr unsigned long EndIteratorValue = BitSetDataCount * 64;
    public:
        BitSet()
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] = 0;
        }

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<N>& operator=(const BitSet<N>& set) = default;
        BitSet<N>& operator=(BitSet<N>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < N, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", N, index, N-1);
            m_data[index / 64] |= ((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < N, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", N, index, N - 1);
            m_data[index / 64] &= ~((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < N, "Setting bit: %i to BitSet<%i> is invalid. Correct index range: 0 .. %i", N, index, N - 1);
            return m_data[index / 64] & ((uint64_t)1u << ((uint64_t)index - (((uint64_t)index / (uint64_t)64ull) * (uint64_t)64ull)));
        }

        bool operator==(const BitSet& set) const
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                if (m_data[i] != set.m_data[i])
                    return false;
            return true;
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] &= other.m_data[i];

            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] |= other.m_data[i];

            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            for (int i = 0; i < BitSetDataCount; ++i)
                m_data[i] ^= other.m_data[i];

            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] &= other.m_data[i];

            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] |= other.m_data[i];

            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;

            for (int i = 0; i < BitSetDataCount; ++i)
                res.m_data[i] ^= other.m_data[i];

            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint64_t,                   // value_type
            uint64_t,                   // difference_type
            const uint64_t*,            // pointer
            uint64_t>                    // reference
        {
            Iterator(const BitSet* set, uint64_t index)
                : m_set{ set }
                , m_block{ 0u }
                , m_index{ index }
                , m_indexInternal{ static_cast<unsigned long>(index) }
            {
                if (index == EndIteratorValue)
                    return;
                this->operator++();
            }

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                while (m_block < BitSetDataCount)
                {
                    unsigned long long mask = m_set->m_data[m_block];
                    mask &= mask64(m_indexInternal);

                    if (_BitScanForward64(&m_indexInternal, mask))
                    {
                        m_index = (m_block * 64) + m_indexInternal;
                        ++m_indexInternal;
                        return *this;
                    }
                    ++m_block;
                }
                m_index = EndIteratorValue;
                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const BitSet* m_set;
            uint64_t m_block;
            uint64_t m_index;
            unsigned long m_indexInternal;
        };

        Iterator begin() const
        {
            return Iterator(this, 0);
        }
        Iterator end() const
        {
            return Iterator(this, EndIteratorValue);
        }
    private:
        uint64_t m_data[BitSetDataCount];
    };

    template<>
    class BitSet<8>
    {
        static constexpr unsigned long EndIteratorValue = 8;
    public:
        BitSet()
            : m_data{ 0u }
        {}

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<8>& operator=(const BitSet<8>& set) = default;
        BitSet<8>& operator=(BitSet<8>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < 8, "Setting bit: %i to BitSet<8> is invalid. Correct index range: 0 .. 7", index);
            m_data |= (uint8_t)1u << (uint8_t)index;
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < 8, "Clearing bit: %i to BitSet<8> is invalid. Correct index range: 0 .. 7", index);
            m_data &= ~((uint8_t)1u << (uint8_t)index);
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < 8, "Clearing bit: %i to BitSet<8> is invalid. Correct index range: 0 .. 7", index);
            return m_data & ((uint8_t)1u << (uint8_t)index);
        }

        bool operator==(const BitSet& set) const
        {
            return m_data == set.m_data;
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            m_data &= other.m_data;
            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            m_data |= other.m_data;
            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            m_data ^= other.m_data;
            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data &= other.m_data;
            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data |= other.m_data;
            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data ^= other.m_data;
            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint8_t,                   // value_type
            uint8_t,                   // difference_type
            const unsigned long*,            // pointer
            uint8_t>                    // reference
        {
            Iterator(const uint8_t* data, uint8_t index)
                : m_data{ data }
            {
                if (index == EndIteratorValue)
                {
                    m_index = EndIteratorValue;
                    return;
                }

                unsigned long m = *m_data & mask8(index);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;
            }

            reference operator*() const { return (uint8_t)m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                unsigned long long m = *m_data & mask8(m_indexMask);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;

                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const uint8_t* m_data;
            unsigned long m_index;
            unsigned long m_indexMask;
        };

        Iterator begin() const
        {
            return Iterator(&m_data, 0);
        }
        Iterator end() const
        {
            return Iterator(&m_data, EndIteratorValue);
        }
    private:
        uint8_t m_data;
    };

    template<>
    class BitSet<16>
    {
        static constexpr unsigned long EndIteratorValue = 16;
    public:
        BitSet()
            : m_data{ 0u }
        {}

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<16>& operator=(const BitSet<16>& set) = default;
        BitSet<16>& operator=(BitSet<16>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < 16, "Setting bit: %i to BitSet<16> is invalid. Correct index range: 0 .. 15", index);
            m_data |= (uint16_t)1u << (uint16_t)index;
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < 16, "Setting bit: %i to BitSet<16> is invalid. Correct index range: 0 .. 15", index);
            m_data &= ~((uint16_t)1u << (uint16_t)index);
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < 16, "Setting bit: %i to BitSet<16> is invalid. Correct index range: 0 .. 15", index);
            return m_data & ((uint16_t)1u << (uint16_t)index);
        }

        bool operator==(const BitSet& set) const
        {
            return m_data == set.m_data;
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            m_data &= other.m_data;
            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            m_data |= other.m_data;
            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            m_data ^= other.m_data;
            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data &= other.m_data;
            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data |= other.m_data;
            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data ^= other.m_data;
            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint16_t,                   // value_type
            uint16_t,                   // difference_type
            const unsigned long*,            // pointer
            uint16_t>                    // reference
        {
            Iterator(const uint16_t* data, uint16_t index)
                : m_data{ data }
            {
                if (index == EndIteratorValue)
                {
                    m_index = EndIteratorValue;
                    return;
                }

                unsigned long m = *m_data & mask16(index);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;
            }

            reference operator*() const { return (uint16_t)m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                unsigned long long m = *m_data & mask16(m_indexMask);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;

                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const uint16_t* m_data;
            unsigned long m_index;
            unsigned long m_indexMask;
        };

        Iterator begin() const
        {
            return Iterator(&m_data, 0);
        }
        Iterator end() const
        {
            return Iterator(&m_data, EndIteratorValue);
        }
    private:
        uint16_t m_data;
    };

    template<>
    class BitSet<32>
    {
        static constexpr unsigned long EndIteratorValue = 32;
    public:
        BitSet()
            : m_data{ 0u }
        {}

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<32>& operator=(const BitSet<32>& set) = default;
        BitSet<32>& operator=(BitSet<32>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < 32, "Setting bit: %i to BitSet<32> is invalid. Correct index range: 0 .. 32", index);
            m_data |= (uint32_t)1u << (uint32_t)index;
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < 32, "Setting bit: %i to BitSet<32> is invalid. Correct index range: 0 .. 32", index);
            m_data &= ~((uint32_t)1u << (uint32_t)index);
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < 32, "Setting bit: %i to BitSet<32> is invalid. Correct index range: 0 .. 32", index);
            return m_data & ((uint32_t)1u << (uint32_t)index);
        }

        bool operator==(const BitSet& set) const
        {
            return m_data == set.m_data;
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            m_data &= other.m_data;
            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            m_data |= other.m_data;
            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            m_data ^= other.m_data;
            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data &= other.m_data;
            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data |= other.m_data;
            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data ^= other.m_data;
            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint32_t,                   // value_type
            uint32_t,                   // difference_type
            const unsigned long*,            // pointer
            uint32_t>                    // reference
        {
            Iterator(const uint32_t* data, uint32_t index)
                : m_data{ data }
            {
                if (index == EndIteratorValue)
                {
                    m_index = EndIteratorValue;
                    return;
                }

                unsigned long m = *m_data & mask32(index);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;
            }

            reference operator*() const { return (uint32_t)m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                unsigned long long m = *m_data & mask32(m_indexMask);
                if (_BitScanForward(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;

                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const uint32_t* m_data;
            unsigned long m_index;
            unsigned long m_indexMask;
        };

        Iterator begin() const
        {
            return Iterator(&m_data, 0);
        }
        Iterator end() const
        {
            return Iterator(&m_data, EndIteratorValue);
        }
    private:
        uint32_t m_data;
    };

    template<>
    class BitSet<64>
    {
        static constexpr unsigned long EndIteratorValue = 64;
    public:
        BitSet()
            : m_data{ 0u }
        {}

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<64>& operator=(const BitSet<64>& set) = default;
        BitSet<64>& operator=(BitSet<64>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < 64, "Setting bit: %i to BitSet<64> is invalid. Correct index range: 0 .. 63", index);
            m_data |= (uint64_t)1u << (uint64_t)index;
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < 64, "Setting bit: %i to BitSet<64> is invalid. Correct index range: 0 .. 63", index);
            m_data &= ~((uint64_t)1u << (uint64_t)index);
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < 64, "Setting bit: %i to BitSet<64> is invalid. Correct index range: 0 .. 63", index);
            return m_data & ((uint64_t)1u << (uint64_t)index);
        }

        bool operator==(const BitSet& set) const
        {
            return m_data == set.m_data;
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            m_data &= other.m_data;
            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            m_data |= other.m_data;
            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            m_data ^= other.m_data;
            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data &= other.m_data;
            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data |= other.m_data;
            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data ^= other.m_data;
            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint64_t,                   // value_type
            uint64_t,                   // difference_type
            const unsigned long*,            // pointer
            uint64_t>                    // reference
        {
            Iterator(const uint64_t* data, uint64_t index)
                : m_data{ data }
            {
                if (index == EndIteratorValue)
                {
                    m_index = EndIteratorValue;
                    return;
                }
                
                unsigned long long m = *m_data & mask64(index);
                if (_BitScanForward64(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;
            }

            reference operator*() const
            {
                return (uint64_t)m_index;
            }
            pointer operator->()
            {
                return &m_index;
            }
            Iterator& operator++()
            {
                unsigned long long m = *m_data & mask64(m_indexMask);
                if (_BitScanForward64(&m_index, m))
                    m_indexMask = m_index + 1;
                else
                    m_index = EndIteratorValue;
                    
                return *this;
            }
            Iterator operator++(int hemmetti)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            friend bool operator== (const Iterator& a, const Iterator& b)
            {
                return a.m_index == b.m_index;
            };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const uint64_t* m_data;
            unsigned long m_index;
            unsigned long m_indexMask;
        };

        Iterator begin() const
        {
            return Iterator(&m_data, 0);
        }
        Iterator end() const
        {
            return Iterator(&m_data, EndIteratorValue);
        }
    private:
        uint64_t m_data;
    };

    template<>
    class BitSet<128>
    {
        static constexpr unsigned long EndIteratorValue = 128;
    public:
        BitSet()
            : m_data{ 0u }
        {}

        BitSet(const BitSet&) = default;
        BitSet(BitSet&&) = default;
        BitSet<128>& operator=(const BitSet<128>& set) = default;
        BitSet<128>& operator=(BitSet<128>&&) = default;

        void set(int index)
        {
            ASSERT(index >= 0 && index < 128, "Setting bit: %i to BitSet<128> is invalid. Correct index range: 0 .. 127", index);
            index < 64 ? 
                m_data[0] |= (uint64_t)1u << (uint64_t)index :
                m_data[1] |= (uint64_t)1u << (uint64_t)(index-64);
        }

        void clear(int index)
        {
            ASSERT(index >= 0 && index < 128, "Setting bit: %i to BitSet<128> is invalid. Correct index range: 0 .. 127", index);
            index < 64 ?
                m_data[0] &= ~((uint64_t)1u << (uint64_t)index) :
                m_data[1] &= ~((uint64_t)1u << (uint64_t)(index - 64));
        }

        bool get(int index) const
        {
            ASSERT(index >= 0 && index < 128, "Setting bit: %i to BitSet<128> is invalid. Correct index range: 0 .. 127", index);
            return index < 64 ?
                m_data[0] & ((uint64_t)1u << (uint64_t)index) :
                m_data[0] & ((uint64_t)1u << (uint64_t)(index-64));
        }

        bool operator==(const BitSet& set) const
        {
            return m_data[0] == set.m_data[0] &&
                   m_data[1] == set.m_data[1];
        }

        bool operator!=(const BitSet& set) const
        {
            return !(*this == set);
        }

        BitSet& operator&=(const BitSet& other) noexcept
        {
            m_data[0] &= other.m_data[0];
            m_data[1] &= other.m_data[1];
            return *this;
        }
        BitSet& operator|=(const BitSet& other) noexcept
        {
            m_data[0] |= other.m_data[0];
            m_data[1] |= other.m_data[1];
            return *this;
        }
        BitSet& operator^=(const BitSet& other) noexcept
        {
            m_data[0] ^= other.m_data[0];
            m_data[1] ^= other.m_data[1];
            return *this;
        }

        BitSet operator&(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data[0] &= other.m_data[0];
            res.m_data[1] &= other.m_data[1];
            return res;
        }
        BitSet operator|(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data[0] |= other.m_data[0];
            res.m_data[1] |= other.m_data[1];
            return res;
        }
        BitSet operator^(const BitSet& other) const noexcept
        {
            BitSet res = *this;
            res.m_data[0] ^= other.m_data[0];
            res.m_data[1] ^= other.m_data[1];
            return res;
        }

    public:
        struct Iterator : public std::iterator<
            std::forward_iterator_tag,  // iterator_category
            uint64_t,                   // value_type
            uint64_t,                   // difference_type
            const uint64_t*,            // pointer
            uint64_t>                    // reference
        {
            Iterator(const BitSet* set, uint64_t index)
                : m_set{ set }
                , m_block{ 0u }
                , m_index{ index }
                , m_indexInternal{ static_cast<unsigned long>(index) }
            {
                if (index == EndIteratorValue)
                    return;
                this->operator++();
            }

            reference operator*() const { return m_index; }
            pointer operator->() { return &m_index; }
            Iterator& operator++()
            {
                while (m_block < 2)
                {
                    unsigned long long mask = m_set->m_data[m_block];
                    mask &= mask64(m_indexInternal);

                    if (_BitScanForward64(&m_indexInternal, mask))
                    {
                        m_index = (m_block * 64) + m_indexInternal;
                        ++m_indexInternal;
                        return *this;
                    }
                    ++m_block;
                    m_indexInternal = 0;
                }
                m_index = EndIteratorValue;
                return *this;
            }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_index == b.m_index; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_index != b.m_index; };
            friend bool operator< (const Iterator& a, const Iterator& b) { return a.m_index < b.m_index; };

        private:
            const BitSet* m_set;
            uint64_t m_block;
            uint64_t m_index;
            unsigned long m_indexInternal;
        };

        Iterator begin() const
        {
            return Iterator(this, 0);
        }
        Iterator end() const
        {
            return Iterator(this, EndIteratorValue);
        }
    private:
        uint64_t m_data[2];
    };
}
