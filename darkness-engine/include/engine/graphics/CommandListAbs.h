#pragma once

#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/Format.h"
#include "engine/graphics/Semaphore.h"
#include "containers/vector.h"
#include "containers/memory.h"

namespace engine
{
    class Device;
    class Texture;
    class Buffer;
    class TextureSRV;
    class TextureUAV;
    class TextureRTV;
    class BufferSRV;
    class BufferUAV;
    class BufferUAVOwner;
    struct CorePipelines;

    struct CommandListAbs
    {
        const Device* m_device;
        const char* m_name;
        CommandListType m_type;
        GraphicsApi m_api;

        engine::shared_ptr<CorePipelines> m_corePipelines;

        engine::vector<BufferUAVOwner> m_debugBuffers;

        engine::vector<Format> m_lastSetRTVFormats;
        Format m_lastSetDSVFormat;
		unsigned int msaaCount;
		unsigned int msaaQuality;

        unsigned long long m_submitFenceValue;

        engine::shared_ptr<Semaphore> commandListSemaphore;
    };
}
