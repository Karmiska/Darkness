#pragma once

#include <string>
#include "containers/vector.h"

namespace serialization
{
    class Stream
    {
    public:
        Stream() {};
        Stream(const void* data, size_t bytes)
        {
            m_data.resize(bytes);
            memcpy(&m_data[0], data, bytes);
        }

        friend Stream& operator<<(Stream& stream, const int& value);
        friend Stream& operator>>(Stream& stream,       int& value);

        void write(const char* data, size_t size)
        {
            if (m_writeoffset + size > m_data.size())
            {
                m_data.resize(m_writeoffset + size);
            }
            memcpy(&m_data[m_writeoffset], data, size);
            m_writeoffset += size;
        };

        size_t read(char* data, size_t size)
        {
            size_t dataAvailable = m_data.size() - m_readOffset;
            size_t bytesRead = size < dataAvailable ? size : dataAvailable;
            memcpy(data, &m_data[m_readOffset], bytesRead);
            return bytesRead;
        };

        enum class Whence
        {
            Begin,
            Current,
            End
        };

        void seekRead(Whence whence, size_t howMuch)
        {
            switch (whence)
            {
                case Whence::Begin:
                {
                    m_readOffset = howMuch;
                    break;
                }
                case Whence::Current:
                {
                    m_readOffset += howMuch;
                    break;
                }
                case Whence::End:
                {
                    m_readOffset = m_data.size() - howMuch;
                    break;
                }
            }
        }

        size_t tellRead() const
        {
            return m_readOffset;
        }

        void seekWrite(Whence whence, size_t howMuch)
        {
            switch (whence)
            {
                case Whence::Begin:
                {
                    m_writeoffset = howMuch;
                    break;
                }
                case Whence::Current:
                {
                    m_writeoffset += howMuch;
                    break;
                }
                case Whence::End:
                {
                    m_writeoffset = m_data.size() - howMuch;
                    break;
                }
            }
        }

        size_t tellWrite() const
        {
            return m_writeoffset;
        }

        const char* data() const
        {
            return m_data.data();
        }

        char* data()
        {
            return m_data.data();
        }

        size_t size() const
        {
            return m_data.size();
        }

    private:
        size_t m_writeoffset = 0;
        size_t m_readOffset = 0;
        engine::vector<char> m_data;
    };

    Stream& operator<<(Stream& stream, const int& value);
    Stream& operator>>(Stream& stream, int& value);
    Stream& operator<<(Stream& stream, const float& value);
    Stream& operator>>(Stream& stream, float& value);
}
