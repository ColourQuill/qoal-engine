#pragma once

namespace vkr {
    class Render {
        public:
            Render(Swapchain& s, CommandPool& c) : swapchain{s}, commandPool{c} {

            }
            ~Render() {
                swapchain.DestroySyncObjects();
            }

            VkCommandBuffer BeginFrame() {
                if (isFrameStarted == true) {
                    throw std::runtime_error("Cannot begin frame while already in progress.");
                }

                auto result = swapchain.AcquireNextImage(&swapchain.GetCurrentImageIndex());
                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    swapchain.RecreateSwapchain();
                    return nullptr;
                }

                if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    throw std::runtime_error("Failed to acquire swap chain image.");
                }

                isFrameStarted = true;

                auto commandBuffer = commandPool.GetCurrentCommandBuffer();
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = 0;
                beginInfo.pInheritanceInfo = nullptr;

                if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to begin frame.");
                }

                return commandBuffer;
            }

            void BeginSwapchainRenderpass(VkCommandBuffer commandBuffer) {
                if (isFrameStarted != true) {
                    throw std::runtime_error("Cannot begin swapchain render pass if frame is not in progress.");
                }
                if (commandBuffer != commandPool.GetCurrentCommandBuffer()) {
                    throw std::runtime_error("Cannot begin render pass on command buffer from a different frame.");
                }

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = swapchain.GetRenderPass();
                renderPassInfo.framebuffer = swapchain.GetFramebuffer(swapchain.GetCurrentImageIndex());

                renderPassInfo.renderArea.offset = {0,0};
                renderPassInfo.renderArea.extent = swapchain.GetExtent();

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0}; // Might need to remove this
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(swapchain.GetExtent().width);
                viewport.height = static_cast<float>(swapchain.GetExtent().height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                VkRect2D scissor{ {0,0}, swapchain.GetExtent() };
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            }

            void EndSwapchainRenderpass(VkCommandBuffer commandBuffer) {
                if (isFrameStarted != true) {
                    throw std::runtime_error("Cannot end swapchain render pass while frame is not in progress.");
                }
                if (commandBuffer != commandPool.GetCurrentCommandBuffer()) {
                    throw std::runtime_error("Cannot end render pass on command buffer from a different frame.");
                }
                vkCmdEndRenderPass(commandBuffer);
            }

            void EndFrame() {
                if (isFrameStarted != true) {
                    throw std::runtime_error("Cannot end frame while frame is not in progress.");
                }
                auto commandBuffer = commandPool.GetCurrentCommandBuffer();
                if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to record command buffer.");
                }

                auto result = swapchain.SubmitCommandBuffers(&commandBuffer, &swapchain.GetCurrentImageIndex());
                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || swapchain.GetWindow().WasWindowResized()) {
                    swapchain.GetWindow().ResetWindowResizedFlag();
                    swapchain.RecreateSwapchain();
                }
                else if (result != VK_SUCCESS) {
                    throw std::runtime_error("Failed to present swap chain image.");
                }

                isFrameStarted = false;
                swapchain.GetCurrentFrameIndex() = (swapchain.GetCurrentFrameIndex() + 1) % swapchain.MAX_FRAMES_IN_FLIGHT;
            }
        private:
            Swapchain& swapchain;
            CommandPool& commandPool;

            bool isFrameStarted{false};
    };
}