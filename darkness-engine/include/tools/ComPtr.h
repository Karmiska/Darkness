#pragma once

#include "tools/Debug.h"

namespace tools
{
    template <typename Interface>
    class RemoveAddReleaseRef : public Interface
    {
        unsigned long __stdcall AddRef();
        unsigned long __stdcall Release();
    };

    template <typename Interface>
    class ComPtr
    {
    public:
        ComPtr() noexcept = default;

        RemoveAddReleaseRef<Interface>* operator->() const noexcept
        {
            return static_cast<RemoveAddReleaseRef<Interface>*>(m_ptr);
        }

        ComPtr(ComPtr const& ptr) noexcept
            : m_ptr{ ptr.m_ptr }
        {
            InternalAddRef();
        }

        /*
        template <typename T>
        ComPtr(ComPtr<T> const& ptr) noexcept
            : m_ptr{ ptr.m_ptr }
        {
            InternalAddRef();
        }

        ComPtr(ComPtr&& ptr) noexcept
            : m_ptr{ ptr.m_ptr }
        {
            ptr.m_ptr = nullptr;
        }

        template <typename T>
        ComPtr(ComPtr<T>&& ptr) noexcept
            : m_ptr{ ptr.m_ptr }
        {
            ptr.m_ptr = nullptr;
        }

        ComPtr& operator=(ComPtr const& other) noexcept
        {
            InternalCopy(other.m_ptr);
            return *this;
        }

        template <typename T>
        ComPtr& operator=(ComPtr<T> const& other) noexcept
        {
            InternalCopy(other.m_ptr);
            return *this;
        }

        template <typename T>
        ComPtr& operator=(ComPtr<T>&& other) noexcept
        {
            InternalMove(other);
            return *this;
        }
        */

        ~ComPtr()
        {
            InternalRelease();
        }

        void Swap(ComPtr& other) noexcept
        {
            Interface* temp = m_ptr;
            m_ptr = other.m_ptr;
            other.m_ptr = temp;
        }

        /*
        explicit operator bool() const noexcept
        {
            return nullptr != m_ptr;
        }
        */

        void Reset() noexcept
        {
            InternalRelease();
        }

        Interface* Get() const noexcept
        {
            return m_ptr;
        }

        /*Interface* Detach() noexcept
        {
            Interface* temp = m_ptr;
            m_ptr = nullptr;
            return temp;
        }

        void Copy(Interface* other) noexcept
        {
            InternalCopy(other);
        }*/

        void Attach(Interface* other) noexcept
        {
            InternalRelease();
            m_ptr = other;
        }

        Interface** GetAddressOf() noexcept
        {
            ASSERT(m_ptr == nullptr);
            return &m_ptr;
        }

        void CopyTo(Interface** other) noexcept
        {
            InternalAddRef();
            *other = m_ptr;
        }
        
        template <typename T>
        ComPtr<T> As() const noexcept
        {
            ComPtr<T> temp;
            m_ptr->QueryInterface(temp.GetAddressOf());
            return temp;
        }
        
    private:
        Interface* m_ptr = nullptr;

        void InternalAddRef() const noexcept
        {
            if (m_ptr)
            {
                m_ptr->AddRef();
            }
        }

        void InternalRelease() noexcept
        {
            Interface* temp = m_ptr;
            if (temp)
            {
                m_ptr = nullptr;
                temp->Release();
            }
        }

        /*void InternalCopy(Interface* other) noexcept
        {
            if (m_ptr != other)
            {
                InternalRelease();
                m_ptr = other;
                InternalAddRef();
            }
        }

        template <typename T>
        void InternalMove(ComPtr<T>& other) noexcept
        {
            if (m_ptr != other.m_ptr)
            {
                InternalRelease();
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
        }*/
    };

    /*template <typename Interface>
    void swap(ComPtr<Interface>& left, ComPtr<Interface>& right) noexcept
    {
        left.Swap(right);
    }*/

}
