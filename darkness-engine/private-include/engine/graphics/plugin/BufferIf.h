#pragma once

namespace engine
{
    enum class Format;

    namespace implementation
    {
        class BufferImplIf
        {
        public:
            virtual ~BufferImplIf() {};
        };

        class PixelBufferImplIf
        {
        public:
            virtual ~PixelBufferImplIf() {};

            virtual Format format() const = 0;
        };

        class ColorBufferImplIf
        {
        public:
            virtual ~ColorBufferImplIf() {};
        };

        class DepthBufferImplIf
        {
        public:
            virtual ~DepthBufferImplIf() {};
        };

        class ShadowBufferImplIf
        {
        public:
            virtual ~ShadowBufferImplIf() {};
        };

        class ByteAddressBufferImplIf
        {
        public:
            virtual ~ByteAddressBufferImplIf() {};
        };

        class StructuredBufferImplIf
        {
        public:
            virtual ~StructuredBufferImplIf() {};
        };

        class TypedBufferImplIf
        {
        public:
            virtual ~TypedBufferImplIf() {};
        };
    }
}
