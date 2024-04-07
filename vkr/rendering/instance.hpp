#pragma once

// std
#include <vector>
#include <iostream>
#include <stdexcept>

namespace vkr {
    class Instance {
        public:
            Instance(ValidationLayers& v) : validationLayers{v} {
                CreateInstance();
            }

            ~Instance() {
                vkDestroyInstance(instance, nullptr);
            }

            void CreateInstance() {
                if (!validationLayers.ValidationLayersSupport()) {
                    throw std::runtime_error("Validation layers requested, but are not available.");
                }

                VkApplicationInfo appInfo{};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "Qoal Engine";
                appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
                appInfo.pEngineName = "Qoal Engine";
                appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
                appInfo.apiVersion = VK_API_VERSION_1_0;

                VkInstanceCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;

                auto extensions = GetRequiredExtensions();
                createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
                if (validationLayers.ValidationLayersSupport()) {
                    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.GetValidationLayers().size());
                    createInfo.ppEnabledLayerNames = validationLayers.GetValidationLayers().data();

                    debugCreateInfo = {};
                    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                    debugCreateInfo.pfnUserCallback = DebugCallback;
                    debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
                } else {
                    createInfo.enabledLayerCount = 0;

                    createInfo.pNext = nullptr;
                }

                if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create instance!");
                }

            }
            
            static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                VkDebugUtilsMessageTypeFlagsEXT messsageType, 
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                void* pUserData
            ) {
                std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

                return VK_FALSE;
            }

            std::vector<const char*> GetRequiredExtensions() {
                uint32_t glfwExtensionCount = 0;
                const char** glfwExtensions;
                glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

                std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

                if (validationLayers.ValidationLayersSupport()) {
                    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }

                return extensions;
            }

            VkInstance& GetInstance() {
                return instance;
            }
        private:
            ValidationLayers& validationLayers;

            VkInstance instance;
    };
}