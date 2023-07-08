#include "engine/FrameCpuCapturer.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    FrameCpuCapturer::FrameCpuCapturer(Device& device)
        : m_device{ device }
        , m_queue{ device.queue() }
        , m_taskIndex{ 0 }
        , m_lock{}
        , m_frame{}
    {
        m_lastCaptureTime = std::chrono::high_resolution_clock::now();
        m_frameDuration = 1000.0f / static_cast<float>(FrameCpuCaptureFPSLimit);
        m_frame.resize(4);
        for(int i = 0; i < m_frame.size(); ++i)
            m_tasks.emplace_back(engine::make_unique<CaptureTask>(device, m_queue, m_frame[i], m_lock));
    }

    void FrameCpuCapturer::captureFrame(TextureSRV frame)
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto sinceLastCapture = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_lastCaptureTime).count()) / 1000000.0;
        if (sinceLastCapture >= m_frameDuration)
        {
            m_lastCaptureTime = now;

            CaptureTask& task = *m_tasks[m_taskIndex];
            task.setTask(m_device, frame);

            ++m_taskIndex;
            if (m_taskIndex > m_tasks.size() - 1)
                m_taskIndex = 0;
        }
    }

    CpuTexture FrameCpuCapturer::latestFrame()
    {
        CpuTexture res;
        {
            std::lock_guard<std::mutex> lock(m_lock);
            //uint64_t lastFence = 0;
            //int fenceIndex = 0;
            //
            //for (int i = 0; i < m_frame.size(); ++i)
            //{
            //    uint64_t fenceValue = m_tasks[i]->lastFenceUpdated().load();
            //    if (fenceValue > lastFence)
            //        fenceIndex = i;
            //}
            //res = m_frame[fenceIndex];

            size_t latestIndex;
            if(m_taskIndex == 0)
                latestIndex = m_frame.size() - 1;
            else
                latestIndex = m_taskIndex - 1;
            res = m_frame[latestIndex];
            
        }
        return res;
    }

    FrameCpuCapturer::CaptureTask::CaptureTask(
        Device& device, 
        Queue& queue, 
        CpuTexture& dstframe,
        std::mutex& mutex)
        : m_alive{ true }
        , m_queue{ queue }
        , m_fence{ device.createFence("Frame cpu capture task thread") }
        , m_thread{ nullptr }
    {
        m_fence.increaseCPUValue();

        m_thread = engine::make_unique<std::thread>([&]()
        {
            while (m_alive)
            {
                m_waitingOnFence = m_fence.currentCPUValue();
                m_fence.blockUntilSignaled(m_waitingOnFence);
                

                dstframe.width = m_copyDesc.width;
                dstframe.height = m_copyDesc.height;
                dstframe.pitch = m_copyDesc.pitch;
                dstframe.pitchBytes = m_copyDesc.pitchBytes;
                dstframe.format = m_copyDesc.format;
                dstframe.zeroUp = m_copyDesc.zeroUp;
                if(!dstframe.data)
                    dstframe.data = engine::make_shared<vector<uint8_t>>();

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    auto ptr = m_buffer.resource().buffer().map(device);
                    if(dstframe.data->size() < m_copyDesc.bufferSize)
                        dstframe.data->resize(m_copyDesc.bufferSize);
                    memcpy(dstframe.data->data(), ptr, m_copyDesc.bufferSize);
                    m_buffer.resource().buffer().unmap(device);
                }

                m_lastFenceUpdated = m_waitingOnFence;
            }
        });
    }

    void FrameCpuCapturer::CaptureTask::setTask(Device& device, TextureSRV frame)
    {
        while(m_fence.currentCPUValue() != m_waitingOnFence)
            std::this_thread::sleep_for(std::chrono::milliseconds(0));

        m_frame = frame;
        m_copyDesc = device.getTextureBufferCopyDesc(m_frame.width(), m_frame.height(), m_frame.format());

        if (!m_buffer.resource().valid() ||
            ((m_buffer.resource().desc().elements * m_buffer.resource().desc().elementSize) < m_copyDesc.bufferSize))
        {
            m_buffer = device.createBufferSRV(engine::BufferDescription()
                .format(frame.format())
                .elementSize(m_copyDesc.elementSize)
                .elements(m_copyDesc.elements)
                .name("Frame capture")
                .usage(engine::ResourceUsage::GpuToCpu));
        }

        CommandList texcmdb = device.createCommandList("Frame cpu capture worker commandlist");
        texcmdb.copyTexture(frame, m_buffer.resource());
        device.submit(texcmdb);

        auto fenceVal = m_fence.currentCPUValue();
        m_fence.increaseCPUValue();
        m_queue.signal(m_fence, fenceVal);

        
    }

    FrameCpuCapturer::CaptureTask::~CaptureTask()
    {
        m_alive = false;
        m_queue.signal(m_fence, m_fence.currentCPUValue());
        m_thread->join();
    }
}
