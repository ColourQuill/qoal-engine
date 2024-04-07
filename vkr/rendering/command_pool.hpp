#pragma once

namespace vkr {
    class CommandPool {
        public:
            CommandPool(Device& d, Swapchain& s) : device{d}, swapchain{s} {
                CreateCommandPool();
                CreateCommandBuffers();
            }

            ~CommandPool() {
                FreeCommandBuffers();
                vkDestroyCommandPool(device.GetDevice(), commandPool, nullptr);
            }

            void CreateCommandPool() {
                Device::QueueFamilyIndices queueFamilyIndices = device.FindQueueFamilies(device.GetPhysicalDevice());

                VkCommandPoolCreateInfo poolInfo{};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

                if (vkCreateCommandPool(device.GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create command pool.");
                }
            }

            void CreateCommandBuffers() {
                commandBuffers.resize(swapchain.MAX_FRAMES_IN_FLIGHT);

                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

                if (vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate command buffers.");
                }
            }

            VkCommandBuffer BeginSingleTimeCommands() {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandPool = commandPool;
                allocInfo.commandBufferCount = 1;

                VkCommandBuffer commandBuffer;
                vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, &commandBuffer);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                vkBeginCommandBuffer(commandBuffer, &beginInfo);
                return commandBuffer;
            }

            void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
                vkEndCommandBuffer(commandBuffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;

                vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(device.GetGraphicsQueue());

                vkFreeCommandBuffers(device.GetDevice(), commandPool, 1, &commandBuffer);
            }

            void FreeCommandBuffers() {
                vkFreeCommandBuffers(device.GetDevice(), commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
                commandBuffers.clear();
            }

            VkCommandBuffer GetCurrentCommandBuffer() {
                return commandBuffers[swapchain.GetCurrentFrameIndex()];
            }

        private:
            Device& device;
            Swapchain& swapchain;

            VkCommandPool commandPool;

            std::vector<VkCommandBuffer> commandBuffers;
    };
}