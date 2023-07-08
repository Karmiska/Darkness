#pragma once

#include "engine/graphics/ResourceOwners.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/DeviceImplIf.h"
#include "engine/graphics/Device.h"
#include <thread>
#include <mutex>
#include <atomic>

namespace engine
{
    const int FrameCpuCaptureFPSLimit = 60;
    class FrameCpuCapturer
    {
    public:
        FrameCpuCapturer(Device& device);
        void captureFrame(TextureSRV frame);
        CpuTexture latestFrame();
    private:
        class CaptureTask
        {
        public:
            CaptureTask(
                Device& device, 
                Queue& queue, 
                CpuTexture& dstframe,
                std::mutex& mutex);
            ~CaptureTask();

            void setTask(Device& device, TextureSRV frame);

            std::atomic_uint64_t& lastFenceUpdated() { return m_lastFenceUpdated; }
        private:
            volatile bool m_alive;
            Queue& m_queue;
            Fence m_fence;
            TextureSRV m_frame;
            TextureBufferCopyDesc m_copyDesc;
            BufferSRVOwner m_buffer;
            engine::unique_ptr<std::thread> m_thread;
            volatile FenceValue m_waitingOnFence;
            std::atomic_uint64_t m_lastFenceUpdated;
        };

        Device& m_device;
        Queue& m_queue;
        size_t m_taskIndex;
        std::mutex m_lock;
        engine::vector<CpuTexture> m_frame;
        engine::vector<engine::unique_ptr<CaptureTask>> m_tasks;

        std::chrono::steady_clock::time_point m_lastCaptureTime;
        float m_frameDuration;
    };
}
