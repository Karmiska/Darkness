#pragma once
#if 0
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace tools
{
    namespace implementation
    {
        class RefCountBase
        {
        private:
            virtual void destroy() noexcept = 0;
            virtual void deleteThis() noexcept = 0;

            uint32_t m_useCount;
        protected:
            RefCountBase()
                : m_useCount{ 1 }
            {}
        public:
            virtual ~RefCountBase() noexcept {}

            bool _incrementIfNotZero()
            {
                if (m_useCount == 0)
                    return false;
                ++m_useCount;
                return true;
            }

            void _increment()
            {
                ++m_useCount;
            }

            void _decrement()
            {
                --m_useCount;
            }

            uint32_t _useCount() const noexcept
            {
                return m_useCount;
            }

            bool _expired() const noexcept
            {
                return m_useCount == 0;
            }
        };

        template<class T>
        class RefCount : public RefCountBase
        {
        public:
            RefCount(T* px)
                : RefCountBase()
                , m_ptr{ px }
            {}
        private:
            virtual void destroy() noexcept
            {
                delete m_ptr;
            }
            virtual void deleteThis() noexcept
            {
                delete this;
            }
            T* m_ptr;
        };

        template<class T>
        class RefCountedPtrBase
        {
        public:
            constexpr RefCountedPtrBase() noexcept
                : m_ptr{ 0 }
                , m_ref{ 0 }
            {}

            RefCountedPtrBase(RefCountedPtrBase<T>&& val)
                : m_ptr{ val.m_ptr }
                , m_ref{ val.m_ref }
            {
                val.m_ptr = 0;
                val.m_ref = 0;
            }

            template<class T2>
            RefCountedPtrBase(RefCountedPtrBase<T2>&& val)
                : m_ptr{ val.m_ptr }
                , m_ref{ val.m_ref }
            {
                val.m_ptr = 0;
                val.m_ref = 0;
            }

            RefCountedPtrBase& operator=(RefCountedPtrBase<T>&& val)
            {
                assign(std::move(val));
                return *this;
            }

            void _assign(RefCountedPtrBase<T>&& val)
            {
                swap(val);
            }

            uint32_t useCount() const noexcept
            {
                return (m_ref ? m_ref->_useCount() : 0);
            }

            void _swap(RefCountedPtrBase& val) noexcept
            {
                std::swap(m_ref, val.m_ref);
                std::swap(m_ptr, val.m_ptr);
            }

            template<class T2>
            bool ownerBefore(const RefCountedPtrBase<T2>& val) const
            {
                return (m_ref < val.m_ref);
            }

            T* _get() const noexcept
            {
                return m_ptr;
            }

            bool _expired() const noexcept
            {
                return (!m_ref || m_ref->_expired());
            }

            void _decrement()
            {
                if (m_ref != 0)
                    m_ref->_decrement();
            }

            void _reset()
            {
                _reset(0, 0);
            }

            template<class T2>
            void _reset(const RefCountedPtrBase<T2>& val)
            {
                _reset(val.m_ptr, val.m_ref);
            }

            template<class T2>
            void _reset(T* ptr, const RefCountedPtrBase<T2>& val)
            {
                _reset(ptr, val.m_ref);
            }

            void _reset(T* ptr, RefCountBase* val)
            {
                if (val)
                    val->_increment();
                _reset0(ptr, val);
            }

            void _reset0(T* ptr, RefCountBase* other)
            {
                if (m_ref != 0)
                    m_ref->_decrement();
                m_ref = other;
                m_ptr = ptr;
            }

        private:
            T* m_ptr;
            RefCountBase* m_ref;
            template<class T0>
            friend class RefCountedPtrBase;
        };
    }

    template<class T>
    class RefCounted : public implementation::RefCountedPtrBase<T>
    {
    public:
        typedef RefCounted<T> myt;
        typedef implementation::RefCountedPtrBase<T> mybase;

        constexpr RefCounted() noexcept
        {}

        template<class U>
        explicit RefCounted(U* ptr)
        {
            _resetp(ptr);
        }

        constexpr RefCounted(nullptr_t) noexcept
        {}

        template<class T2>
        RefCounted(const RefCounted<T2>& val, T* ptr) noexcept
        {
            this->_resetp(ptr, val);
        }

        RefCounted(const myt& other) noexcept
        {
            this->_reset(other);
        }

        template<class T2, class = std::enable_if<std::is_convertible<T2*, T*>::value>>
        RefCounted(const RefCounted<T2>& other) noexcept
        {
            this->_reset(other);
        }

        RefCounted(myt&& val) noexcept
            : mybase(std::move(val))
        {
        }

        template<class T2, class = std::enable_if<std::is_convertible<T2*, T*>::value>>
        RefCounted(RefCounted<T2>&& other) noexcept
            : mybase(std::move(other))
        {
        }

        myt& operator=(myt&& val) noexcept
        {
            RefCounted(std::move(val)).swap(*this);
            return *this;
        }

        template<class T2>
        myt& operator=(RefCounted<T2>&& val) noexcept
        {
            RefCounted(std::move(val)).swap(*this);
            return *this;
        }

        ~RefCounted() noexcept
        {
            this->_decrement();
        }

        myt& operator=(const myt& val) noexcept
        {
            RefCounted(val).swap(*this);
            return *this;
        }

        template<class T2>
        myt& operator=(const RefCounted<T2>& val) noexcept
        {
            RefCounted(val).swap(*this);
            return *this;
        }

        void reset() noexcept
        {
            RefCounted().swap(*this);
        }

        template<class U>
        void reset(U* ptr)
        {
            RefCounted(ptr).swap(*this);
        }

        void swap(myt& val) noexcept
        {
            this->_swap(val);
        }

        T* get() const noexcept
        {
            return this->_get();
        }

        T* operator->() const noexcept
        {
            return this->_get();
        }

        bool unique() const noexcept
        {
            return this->useCount() == 1;
        }

        explicit operator bool() const noexcept
        {
            return this->_get() != 0;
        }

    private:
        template<class U>
        void _resetp(U* ptr)
        {
            try
            {
                _reset0(ptr, new implementation::RefCount<U>(ptr));
            }
            catch (...)
            {
                delete ptr;
                throw;
            }
        }
    public:
        template<class U>
        void resetp0(U* ptr, implementation::RefCountBase* bptr)
        {
            this->_reset0(ptr, bptr);
        }
    };

    template<class T1, class T2>
    bool operator==(const RefCounted<T1>& left, const RefCounted<T2>& right) noexcept
    {
        return left.get() == right.get();
    }

    template<class T1, class T2>
    bool operator!=(const RefCounted<T1>& left, const RefCounted<T2>& right) noexcept
    {
        return (!(left == right));
    }

}
#endif
