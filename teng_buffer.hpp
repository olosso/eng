#pragma once

#include "teng_device.hpp"
#include <vulkan/vulkan_core.h>

namespace teng {

    class Buffer {
        public:
            Buffer(
                Device& device,
                VkDeviceSize instanceSize,
                uint32_t instanceCount,
                VkBufferUsageFlags usageFlags,
                VkMemoryPropertyFlags memoryPropertyFlags,
                VkDeviceSize minOffsetAlignment = 1); // Vertex and Index Buffers don't have alignment requirements.
            ~Buffer();

            Buffer(const Buffer&) = delete;
            Buffer& operator=(const Buffer&) = delete;

            VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            void unmap();

            void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            void writeToIndex(void* data, int index);
            VkResult flushIndex(int index);
            VkDescriptorBufferInfo descriptorInfoForIndex(int index);
            VkResult invalidateIndex(int index);

            VkBuffer getBuffer() const { return buffer; }
            void* getMappedMemory() const { return mapped; }
            uint32_t getInstanceCount() const { return m_InstanceCount; }
            VkDeviceSize getInstanceSize() const { return m_InstanceSize; }
            VkDeviceSize getAlignmentSize() const { return m_InstanceSize; }
            VkBufferUsageFlags getUsageFlags() const { return m_UsageFlags; }
            VkMemoryPropertyFlags getMemoryPropertyFlags() const { return m_MemoryPropertyFlags; }
            VkDeviceSize getBufferSize() const { return m_BufferSize; }

        private:
            static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

            Device& mr_Device;
            void* mapped = nullptr;
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            VkDeviceSize m_BufferSize;
            uint32_t m_InstanceCount;
            VkDeviceSize m_InstanceSize;
            VkDeviceSize m_AlignmentSize;
            VkBufferUsageFlags m_UsageFlags;
            VkMemoryPropertyFlags m_MemoryPropertyFlags;
    };

}  // namespace lve
