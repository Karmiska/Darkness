#pragma once

struct METAL_CPU_DESCRIPTOR_HANDLE
{
};

struct METAL_GPU_DESCRIPTOR_HANDLE
{
};

namespace engine
{
    enum class DescriptorHandleType;

    namespace implementation
    {
        class DescriptorHandleImpl
        {
        public:
            DescriptorHandleImpl();
            DescriptorHandleImpl(const METAL_CPU_DESCRIPTOR_HANDLE& handle, unsigned int incrementSize);
            DescriptorHandleImpl(const METAL_GPU_DESCRIPTOR_HANDLE& handle, unsigned int incrementSize);
            ~DescriptorHandleImpl();
            
            DescriptorHandleImpl(const DescriptorHandleImpl& handle);
            DescriptorHandleImpl& operator=(const DescriptorHandleImpl& handle);
            DescriptorHandleImpl(DescriptorHandleImpl&& handle);
            DescriptorHandleImpl& operator=(DescriptorHandleImpl&& handle);

            DescriptorHandleType type() const;
            const METAL_CPU_DESCRIPTOR_HANDLE& cpuHandle() const;
            const METAL_GPU_DESCRIPTOR_HANDLE& gpuHandle() const;

            unsigned int incrementSize() const;
            void setIncrementSize(unsigned int size);
            DescriptorHandleImpl& operator++();
            DescriptorHandleImpl operator++(int);
            DescriptorHandleImpl& operator+=(int count);
        private:
            METAL_CPU_DESCRIPTOR_HANDLE* m_cpuHandle;
            METAL_GPU_DESCRIPTOR_HANDLE* m_gpuHandle;
            unsigned int m_incrementSize;
        };
    }
}
