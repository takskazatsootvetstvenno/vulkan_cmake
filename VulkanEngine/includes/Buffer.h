#pragma once

#include "device.h"

namespace sge {

    class Buffer {
    public:
        Buffer(
            Device& device,
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        VkResult map(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(const void* data, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
        VkResult flush(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0) const;
        VkResult invalidate(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0);

        void writeToIndex(const void* data, const int index);
        VkResult flushIndex(const int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(const int index);
        VkResult invalidateIndex(const int index);

        VkBuffer getBuffer() const { return m_buffer; }
        void* getMappedMemory() const { return m_mapped; }
        uint32_t getInstanceCount() const { return m_instanceCount; }
        VkDeviceSize getInstanceSize() const { return m_instanceSize; }
        VkDeviceSize getAlignmentSize() const { return m_instanceSize; }
        VkBufferUsageFlags getUsageFlags() const { return m_usageFlags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_memoryPropertyFlags; }
        VkDeviceSize getBufferSize() const { return m_bufferSize; }

    private:
        static constexpr VkDeviceSize getAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) noexcept;

        Device& m_device;
        void* m_mapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize;
        uint32_t m_instanceCount;
        VkDeviceSize m_instanceSize;
        VkDeviceSize m_alignmentSize;
        VkBufferUsageFlags m_usageFlags;
        VkMemoryPropertyFlags m_memoryPropertyFlags;
    };
}