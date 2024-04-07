#pragma once

// std
#include <vector>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <set>

namespace vkr {
    class Device {
        public:
            Device(Window& w, ValidationLayers& v, Instance& i, Surface& s) : window{w}, instance{i}, validationLayers{v}, surface{s} {
                PickPhysicalDevice();
                CreateLogicalDevice();
            }

            ~Device() {
                vkDestroyDevice(device, nullptr);
            }

            void PickPhysicalDevice() {
                auto inst = instance.GetInstance();

                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(inst, &deviceCount, nullptr);

                if (deviceCount == 0) {
                    throw std::runtime_error("Failed to find GPUs with Vulkan support.");
                }

                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(inst, &deviceCount, devices.data());

                for (const auto& device : devices) {
                    if (isDeviceSuitable(device)) {
                        physicalDevice = device;
                        break;
                    }
                }

                if (physicalDevice == VK_NULL_HANDLE) {
                    throw std::runtime_error("Failed to find a suitable GPU.");
                }
            }

            bool isDeviceSuitable(VkPhysicalDevice d) {
                QueueFamilyIndices indices = FindQueueFamilies(d);

                bool extensionsSupported = CheckDeviceExtensionSupport(d);

                bool swapchainAdequate = false;
                if (extensionsSupported) {
                    SwapchainSupportDetails swapchainSupport = QuerySwapChainSupport(d);
                    swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
                }

                return indices.isComplete() && extensionsSupported && swapchainAdequate; // Maybe implement better suitable device check later https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
            }

            bool CheckDeviceExtensionSupport(VkPhysicalDevice d) {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(d, nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(d, nullptr, &extensionCount, availableExtensions.data());

                std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

                for (const auto& extension : availableExtensions) {
                    requiredExtensions.erase(extension.extensionName);
                }

                return requiredExtensions.empty();
            }

            struct QueueFamilyIndices {
                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;

                bool isComplete() {
                    return graphicsFamily.has_value() && presentFamily.has_value();
                }
            };

            struct SwapchainSupportDetails {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> presentModes;
            };

            SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice d) {
                SwapchainSupportDetails details;

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

            QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice d) {
                QueueFamilyIndices indices;

                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, queueFamilies.data());

                int i = 0;
                for (const auto& queueFamily : queueFamilies) {
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        indices.graphicsFamily = i;
                    }

                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface.GetSurface(), &presentSupport);

                    if (presentSupport) {
                        indices.presentFamily = i;
                    }

                    if (indices.isComplete()) {
                        break;
                    }

                    i++;
                }

                return indices;
            }

            void CreateLogicalDevice() {
                QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

                std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

                float queuePriority = 1.0f;
                for (uint32_t queueFamily : uniqueQueueFamilies) {
                    VkDeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueFamily;
                    queueCreateInfo.queueCount = 1;
                    queueCreateInfo.pQueuePriorities = &queuePriority;
                    queueCreateInfos.push_back(queueCreateInfo);
                }

                VkPhysicalDeviceFeatures deviceFeatures{};

                VkDeviceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                createInfo.pQueueCreateInfos = queueCreateInfos.data();
                createInfo.pEnabledFeatures = &deviceFeatures;

                createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
                createInfo.ppEnabledExtensionNames = deviceExtensions.data();

                if (validationLayers.ValidationLayersSupport()) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetValidationLayers().size());
                    createInfo.ppEnabledLayerNames = validationLayers.GetValidationLayers().data();
                } else {
                    createInfo.enabledLayerCount = 0;
                }

                if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create logical device.");
                }

                vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
                vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
            }

            VkPhysicalDevice& GetPhysicalDevice() {
                return physicalDevice;
            }

            VkDevice& GetDevice() {
                return device;
            }

            VkQueue& GetGraphicsQueue() {
                return graphicsQueue;
            }

            VkQueue& GetPresentQueue() {
                return presentQueue;
            }

        private:
            Window& window;
            ValidationLayers& validationLayers;
            Instance& instance;
            Surface& surface;

            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue;
            VkQueue presentQueue;

            const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
    };
}