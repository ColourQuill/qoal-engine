#pragma once

namespace vkr {
    class Debugger {
        public:
            Debugger(ValidationLayers& v, Instance& i) : validationLayers{v}, instance{i} {
                SetupDebugMessenger();
            }

            ~Debugger() {
                DestroyDebugUtilsMessengerEXT(nullptr);
            }
        private:
            ValidationLayers& validationLayers;
            Instance& instance;

            VkDebugUtilsMessengerEXT debugMessenger;

            void SetupDebugMessenger() {
                if (!validationLayers.ValidationLayersSupport()) return;

                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                PopulateDebugMessengerCreateInfo(createInfo);

                if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to setup debug messenger.");
                }
            }

            VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator) {
                auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance.GetInstance(), "vkCreateDebugUtilsMessengerEXT");
                if (func != nullptr) {
                    return func(instance.GetInstance(), pCreateInfo, pAllocator, &debugMessenger);
                } else {
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
                }
            }

            void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
                createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = DebugCallback;
                createInfo.pUserData = nullptr;
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

            void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance.GetInstance(), "vkDestroyDebugUtilsMessengerEXT");
                if (func != nullptr) {
                    func(instance.GetInstance(), debugMessenger, pAllocator);
                }
            }

    };
}