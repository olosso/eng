#include "teng_buffer.hpp"

/*
 * Encapsulates a Vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "teng_buffer.hpp"

// std
#include <cassert>
#include <cstring>

namespace teng {

    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */
    VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    Buffer::Buffer(
        Device &device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : mr_Device{device},
          m_InstanceCount{instanceCount},
          m_InstanceSize{instanceSize},
          m_UsageFlags{usageFlags},
          m_MemoryPropertyFlags{memoryPropertyFlags} {
        m_AlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        m_BufferSize = m_AlignmentSize * instanceCount;
        device.createBuffer(m_BufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    Buffer::~Buffer() {
        unmap();
        vkDestroyBuffer(mr_Device.device(), buffer, nullptr);
        vkFreeMemory(mr_Device.device(), memory, nullptr);
    }

    /**
    * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
    *
    * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
    * buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the buffer mapping call
    */
    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(mr_Device.device(), memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */
    void Buffer::unmap() {
      if (mapped) {
        vkUnmapMemory(mr_Device.device(), memory);
        mapped = nullptr;
      }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes
     * whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to
     * flush the complete buffer range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */
    void Buffer::writeToBuffer(void *data, VkDeviceSize size,
                               VkDeviceSize offset) {
      assert(mapped && "Cannot copy to unmapped buffer");

      if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, m_BufferSize);
      } else {
        char *memOffset = (char *)mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
      }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass
     * VK_WHOLE_SIZE to flush the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */
    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
      VkMappedMemoryRange mappedRange = {};
      mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      mappedRange.memory = memory;
      mappedRange.offset = offset;
      mappedRange.size = size;
      return vkFlushMappedMemoryRanges(mr_Device.device(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass
     * VK_WHOLE_SIZE to invalidate the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */
    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
      VkMappedMemoryRange mappedRange = {};
      mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      mappedRange.memory = memory;
      mappedRange.offset = offset;
      mappedRange.size = size;
      return vkInvalidateMappedMemoryRanges(mr_Device.device(), 1,
                                            &mappedRange);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */
    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size,
                                                  VkDeviceSize offset) {
      return VkDescriptorBufferInfo{
          buffer,
          offset,
          size,
      };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of
     * index * alignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */
    void Buffer::writeToIndex(void *data, int index) {
        writeToBuffer(data, m_InstanceSize, index * m_AlignmentSize);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it
     * visible to the device
     *
     * @param index Used in offset calculation
     *
     */
    VkResult Buffer::flushIndex(int index) {
        return flush(m_AlignmentSize, index * m_AlignmentSize);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */
    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(m_AlignmentSize, index * m_AlignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */
    VkResult Buffer::invalidateIndex(int index) {
        return invalidate(m_AlignmentSize, index * m_AlignmentSize);
    }

}  // namespace lve
