#pragma once

// std
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

namespace vkr {
    class Swapchain {
        public:
            Swapchain(Window& w,  Surface& s, Device& d) : window{w}, surface{s}, device{d} {
                CreateSwapchain();
                CreateImageViews();
                CreateRenderPass();
                CreateFrameBuffers();
                CreateSyncObjects();
            }

            void RecreateSwapchain() {
                auto extent = window.GetExtent();
                while (extent.width == 0 || extent.height == 0) {
                    extent = window.GetExtent();
                    glfwWaitEvents();
                }

                vkDeviceWaitIdle(device.GetDevice());

                DestroySyncObjects();

                for (auto framebuffer : swapchainFramebuffers) {
                    vkDestroyFramebuffer(device.GetDevice(), framebuffer, nullptr);
                }

                vkDestroyRenderPass(device.GetDevice(), renderpass, nullptr);

                for (auto imageView : swapchainImageViews) {
                    vkDestroyImageView(device.GetDevice(), imageView, nullptr);
                }
                
                vkDestroySwapchainKHR(device.GetDevice(), swapchain, nullptr);

                CreateSwapchain();
                CreateImageViews();
                CreateRenderPass();
                CreateFrameBuffers();
                CreateSyncObjects();
            }

            ~Swapchain() {
                for (auto framebuffer : swapchainFramebuffers) {
                    vkDestroyFramebuffer(device.GetDevice(), framebuffer, nullptr);
                }

                vkDestroyRenderPass(device.GetDevice(), renderpass, nullptr);

                for (auto imageView : swapchainImageViews) {
                    vkDestroyImageView(device.GetDevice(), imageView, nullptr);
                }
                
                vkDestroySwapchainKHR(device.GetDevice(), swapchain, nullptr);
            }

            VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
                for (const auto& availableFormat : availableFormats) {
                    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        return availableFormat;
                    }
                }

                return availableFormats[0];
            }

            VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
                for (const auto& availablePresentMode : availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        return availablePresentMode;
                    }
                }

                return VK_PRESENT_MODE_FIFO_KHR;
            }

            VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                } else {
                    int width, height;
                    glfwGetFramebufferSize(window.getWindow(), &width, &height);

                    VkExtent2D actualExtent = {
                        static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height)
                    };

                    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                    
                    return actualExtent;
                }
            }

            Device::SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice d) {
                Device::SwapchainSupportDetails details;

                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d, surface.GetSurface(), &details.capabilities);

                uint32_t formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(d, surface.GetSurface(), &formatCount, nullptr);

                if (formatCount != 0) {
                    details.formats.resize(formatCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(d, surface.GetSurface(), &formatCount, details.formats.data());
                }

                uint32_t presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(d, surface.GetSurface(), &presentModeCount, nullptr);

                if (presentModeCount != 0) {
                    details.presentModes.resize(presentModeCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(d, surface.GetSurface(), &presentModeCount, details.presentModes.data());
                }

                return details;
            }

            void CreateSwapchain() {
                Device::SwapchainSupportDetails swapchainSupport = QuerySwapChainSupport(device.GetPhysicalDevice());

                VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.formats);
                VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.presentModes);
                VkExtent2D swapchainExtent = ChooseSwapExtent(swapchainSupport.capabilities);

                uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

                if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
                    imageCount = swapchainSupport.capabilities.maxImageCount;
                }

                VkSwapchainCreateInfoKHR createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.surface = surface.GetSurface();
                createInfo.minImageCount = imageCount;
                createInfo.imageFormat = surfaceFormat.format;
                createInfo.imageColorSpace = surfaceFormat.colorSpace;
                createInfo.imageExtent = swapchainExtent;
                createInfo.imageArrayLayers = 1;
                createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

                Device::QueueFamilyIndices indices = device.FindQueueFamilies(device.GetPhysicalDevice());
                uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

                if (indices.graphicsFamily != indices.presentFamily) {
                    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = 2;
                    createInfo.pQueueFamilyIndices = queueFamilyIndices;
                } else {
                    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0;
                    createInfo.pQueueFamilyIndices = nullptr;
                }

                createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
                createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                createInfo.presentMode = presentMode;
                createInfo.clipped = VK_TRUE;
                createInfo.oldSwapchain = VK_NULL_HANDLE; // Later use this to handle window resizing by creating new swapchain

                if (vkCreateSwapchainKHR(device.GetDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create swapchain.");
                }

                vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, nullptr);
                swapchainImages.resize(imageCount);
                vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, swapchainImages.data());

                format = surfaceFormat.format;
                extent = swapchainExtent;
            }

            void CreateImageViews() {
                swapchainImageViews.resize(swapchainImages.size());

                for (size_t i = 0; i < swapchainImages.size(); i++) {
                    VkImageViewCreateInfo createInfo{};
                    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    createInfo.image = swapchainImages[i];
                    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    createInfo.format = format;

                    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    createInfo.subresourceRange.baseMipLevel = 0;
                    createInfo.subresourceRange.levelCount = 1;
                    createInfo.subresourceRange.baseArrayLayer = 0;
                    createInfo.subresourceRange.layerCount = 1;

                    if (vkCreateImageView(device.GetDevice(), &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create image views.");
                    }
                }
            }

            void CreateRenderPass() {
                VkAttachmentDescription colourAttachment{};
                colourAttachment.format = format;
                colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                VkAttachmentReference colourAttachmentRef{};
                colourAttachmentRef.attachment = 0;
                colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                VkSubpassDescription subpass{};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &colourAttachmentRef;

                VkRenderPassCreateInfo renderpassInfo{};
                renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderpassInfo.attachmentCount = 1;
                renderpassInfo.pAttachments = &colourAttachment;
                renderpassInfo.subpassCount = 1;
                renderpassInfo.pSubpasses = &subpass;

                if (vkCreateRenderPass(device.GetDevice(), &renderpassInfo, nullptr, &renderpass) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create render pass.");
                }
            }

            void CreateFrameBuffers() {
                swapchainFramebuffers.resize(swapchainImageViews.size());

                for (size_t i = 0; i < swapchainImageViews.size(); i++) {
                    VkImageView attachments[] = {
                        swapchainImageViews[i]
                    };

                    VkFramebufferCreateInfo framebufferInfo{};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = renderpass;
                    framebufferInfo.attachmentCount = 1;
                    framebufferInfo.pAttachments = attachments;
                    framebufferInfo.width = extent.width;
                    framebufferInfo.height = extent.height;
                    framebufferInfo.layers = 1;

                    if (vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create framebuffer.");
                    }
                }
            }

            void CreateSyncObjects() {
                imageAvailableSemaphores.clear();
                renderFinishedSemaphores.clear();
                inFlightFences.clear();
                imagesInFlight.clear();
                imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
                renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
                inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
                imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                
                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                        vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                        vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create synchronization objects for a frame.");
                    }
                }
                
            }

            void DestroySyncObjects() {
                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    vkWaitForFences(device.GetDevice(), 1, &inFlightFences[i], VK_TRUE, UINT64_MAX);
                    vkDestroySemaphore(device.GetDevice(), imageAvailableSemaphores[i], nullptr);
                    vkDestroySemaphore(device.GetDevice(), renderFinishedSemaphores[i], nullptr);
                    vkDestroyFence(device.GetDevice(), inFlightFences[i], nullptr);
                }
            }

            VkResult AcquireNextImage(uint32_t* imageIndex) {
                vkWaitForFences(device.GetDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
                VkResult result = vkAcquireNextImageKHR(device.GetDevice(), swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
                return result;
            }

            VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
                if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
                    vkWaitForFences(device.GetDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
                }
                imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
                VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                submitInfo.pWaitDstStageMask = waitStages;

                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = buffers;

                VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;

                vkResetFences(device.GetDevice(), 1, &inFlightFences[currentFrame]);
                if (vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to submit draw command buffer.");
                }

                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;

                VkSwapchainKHR swapchains[] = {swapchain};
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapchains;

                presentInfo.pImageIndices = imageIndex;

                auto result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

                currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

                return result;
            }

            VkSwapchainKHR& GetSwapchain() {
                return swapchain;
            }

            std::vector<VkImage> GetSwapchainImages() {
                return swapchainImages;
            }

            std::vector<VkImageView> GetSwapchainImageViews() {
                return swapchainImageViews;
            }

            VkRenderPass& GetRenderPass() {
                return renderpass;
            }

            VkFramebuffer& GetFramebuffer(int index) {
                return swapchainFramebuffers[index];
            }

            VkFormat& GetFormat() {
                return format;
            }

            VkExtent2D& GetExtent() {
                return extent;
            }

            int& GetCurrentFrameIndex() {
                return currentFrameIndex;
            }

            uint32_t& GetCurrentImageIndex() {
                return currentImageIndex;
            }

            Window& GetWindow() {
                return window;
            }

            Device& GetDevice() {
                return device;
            }

            Surface& GetSurface() {
                return surface;
            }

            int MAX_FRAMES_IN_FLIGHT = 2;

        private:
            Window& window;
            Surface& surface;
            Device& device;

            VkSwapchainKHR swapchain;

            VkFormat format;
            VkExtent2D extent;

            std::vector<VkImage> swapchainImages;
            std::vector<VkImageView> swapchainImageViews;

            VkRenderPass renderpass;

            std::vector<VkFramebuffer> swapchainFramebuffers;

            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence> inFlightFences;
            std::vector<VkFence> imagesInFlight;

            size_t currentFrame{0};
            int currentFrameIndex{0};
            uint32_t currentImageIndex;
    };
}