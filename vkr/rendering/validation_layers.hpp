#pragma once

// std
#include <vector>
#include <cstring>

namespace vkr {
    class ValidationLayers {
        public:
            ValidationLayers() {

            }

            bool ValidationLayersSupport() {
                return (enableValidationLayers && CheckValidationLayerSupport());
            }

            const std::vector<const char*>& GetValidationLayers() {
                return validationLayers;
            }

        private:
            #ifdef NDEBUG
            const bool enableValidationLayers = false;
            #else
            const bool enableValidationLayers = true;
            #endif

            const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
            };

            bool CheckValidationLayerSupport() {
                uint32_t layerCount;
                vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

                std::vector<VkLayerProperties> availableLayers(layerCount);
                vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

                for (const char* layerName : validationLayers) {
                    bool layerFound = false;

                    for (const auto& layerProperties : availableLayers) {
                        if (strcmp(layerName, layerProperties.layerName) == 0) {
                            layerFound = true;
                            break;
                        }
                    }

                    if (!layerFound) {
                        return false;
                    }
                }

                return true;
            }
    };
}