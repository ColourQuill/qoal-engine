#pragma once

// std
#include <stdexcept>

namespace vkr {
    class Surface {
        public:
            Surface(Window& w, Instance& i) : window{w}, instance{i} {
                CreateSurface();
            }

            ~Surface() {
                vkDestroySurfaceKHR(instance.GetInstance(), surface, nullptr);
            }

            void CreateSurface() {
                if (glfwCreateWindowSurface(instance.GetInstance(), window.getWindow(), nullptr, &surface) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create window surface.");
                }
            }

            VkSurfaceKHR& GetSurface() {
                return surface;
            }

        private:
            Window& window;
            Instance& instance;

            VkSurfaceKHR surface;
    };
}