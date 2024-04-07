#pragma once

#include "../../structs/vertex.hpp"

#include <memory>

namespace vkr {
    class Buffer {
        public:
            VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
                if (minOffsetAlignment > 0) {
                    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
                }
                return instanceSize;
            }

            uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(device.GetPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                    if ((typeFilter & (1 << i)) &&
                        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                        return i;
                    }
                }
                throw std::runtime_error("Failed to find suitable memory type.");
            }

            Buffer(Device& d, VkDeviceSize is, uint32_t ic, VkBufferUsageFlags uf, VkMemoryPropertyFlags mpf, VkDeviceSize moa) : device{d}, instanceSize{is}, instanceCount{ic}, usageFlags{uf}, memoryPropertyFlags{mpf} {
                alignmentSize = GetAlignment(is, moa);
                bufferSize = alignmentSize * instanceCount;

                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = bufferSize;
                bufferInfo.usage = usageFlags;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                if (vkCreateBuffer(device.GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create buffer.");
                }

                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(device.GetDevice(), buffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, mpf);

                if (vkAllocateMemory(device.GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate buffer memory.");
                }

                vkBindBufferMemory(device.GetDevice(), buffer, memory, 0);
            }

            ~Buffer() {
                Unmap();
                vkDestroyBuffer(device.GetDevice(), buffer, nullptr);
                vkFreeMemory(device.GetDevice(), memory, nullptr);
            }

            VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
                VkResult result = vkMapMemory(device.GetDevice(), memory, offset, size, 0, &mapped);
                return result;
            }
            
            void Unmap() {
                if (mapped) {
                    vkUnmapMemory(device.GetDevice(), memory);
                    mapped = nullptr;
                }
            }

            void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
                if (!mapped) {
                    throw std::runtime_error("Cannot copy to unmapped buffer.");
                }

                if (size == VK_WHOLE_SIZE) {
                    memcpy(mapped, data, bufferSize);
                }
                else {
                    char* memOffset = (char*)mapped;
                    memOffset += offset;
                    memcpy(memOffset, data, size);
                }
            }

            VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
                VkMappedMemoryRange mappedRange = {};
                mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                mappedRange.memory = memory;
                mappedRange.offset = offset;
                mappedRange.size = size;
                return vkFlushMappedMemoryRanges(device.GetDevice(), 1, &mappedRange);
            }

            VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
                VkMappedMemoryRange mappedRange = {};
                mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
                mappedRange.memory = memory;
                mappedRange.offset = offset;
                mappedRange.size = size;
                return vkInvalidateMappedMemoryRanges(device.GetDevice(), 1, &mappedRange);
            }

            VkBuffer& GetBuffer() {
                return buffer;
            }

            VkDeviceMemory& GetBufferMemory() {
                return memory;
            }

            uint32_t& GetInstanceCount() {
                return instanceCount;
            }

            VkDeviceSize& GetInstanceSize() {
                return instanceSize;
            }

            VkDeviceSize& GetAlignmentSize() {
                return alignmentSize;
            }

            VkDeviceSize& GetBufferSize() {
                return bufferSize;
            }

        private:
            Device& device;

            void* mapped = nullptr;
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            VkDeviceSize bufferSize;
            uint32_t instanceCount;
            VkDeviceSize instanceSize;
            VkDeviceSize alignmentSize;
            VkBufferUsageFlags usageFlags;
            VkMemoryPropertyFlags memoryPropertyFlags;

            // Only if buffer is vertex or index buffer
            std::shared_ptr<std::vector<Vertex2D>> vertices2D;
            std::shared_ptr<std::vector<Vertex3D>> vertices3D;
            std::shared_ptr<std::vector<uint32_t>> indices;

    };
    class BufferManager {
        public:
            BufferManager(Device& d, CommandPool& c) : device{d}, command_pool{c} {

            }

            std::shared_ptr<Buffer> CreateBuffer(VkDeviceSize& instanceSize, uint32_t& instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1) {
                std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(device, instanceSize, instanceCount, usageFlags, memoryPropertyFlags, minOffsetAlignment);
                return buffer;
            }

            void AddBufferToBufferPool(std::shared_ptr<Buffer> buffer) {
                buffer_pool.push_back(buffer);
            }

            void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) {
                VkCommandBuffer commandBuffer = command_pool.BeginSingleTimeCommands();

                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = srcOffset;
                copyRegion.dstOffset = dstOffset;
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

                command_pool.EndSingleTimeCommands(commandBuffer);
            }

            void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount){
                VkCommandBuffer commandBuffer = command_pool.BeginSingleTimeCommands();

                VkBufferImageCopy region{};
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = layerCount;

                region.imageOffset = {0,0,0};
                region.imageExtent = {width, height, 1};

                vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                command_pool.EndSingleTimeCommands(commandBuffer);
            }

            std::shared_ptr<Buffer> CreateVertexBuffer(const std::vector<Vertex3D>& vertices) {
                uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
                if (vertexCount < 3) {
                    throw std::runtime_error("Vertex count must be atleast 3.");
                }
                VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
                VkDeviceSize vertexSize = sizeof(vertices[0]);

                std::shared_ptr<Buffer> stagingBuffer = CreateBuffer(vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                stagingBuffer->Map();
                stagingBuffer->WriteToBuffer((void*)vertices.data());

                std::shared_ptr<Buffer> vertexBuffer = CreateBuffer(vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                CopyBuffer(stagingBuffer->GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);

                AddBufferToBufferPool(vertexBuffer);

                return vertexBuffer;
            }
            std::shared_ptr<Buffer> CreateVertexBuffer(const std::vector<Vertex2D>& vertices) {
                uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
                if (vertexCount < 3) {
                    throw std::runtime_error("Vertex count must be atleast 3.");
                }
                VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
                VkDeviceSize vertexSize = sizeof(vertices[0]);

                std::shared_ptr<Buffer> stagingBuffer = CreateBuffer(vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                stagingBuffer->Map();
                stagingBuffer->WriteToBuffer((void*)vertices.data());

                std::shared_ptr<Buffer> vertexBuffer = CreateBuffer(vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                CopyBuffer(stagingBuffer->GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);

                AddBufferToBufferPool(vertexBuffer);

                return vertexBuffer;
            }

            std::vector<std::shared_ptr<Buffer>>& GetBufferPool() {
                return buffer_pool;
            }
        private:
            Device& device;
            CommandPool& command_pool;

            std::vector<std::shared_ptr<Buffer>> buffer_pool;
    };
}