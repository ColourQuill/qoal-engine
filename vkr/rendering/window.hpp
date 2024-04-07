#pragma once

#include "GLFW/glfw3.h"

namespace vkr {
    class Window {
        public:
            Window() {
                glfwInit();

                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Change later

                window = glfwCreateWindow(WIDTH, HEIGHT, "Qoal Engine", nullptr, nullptr);
                glfwSetWindowUserPointer(window, this);
                glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
            }

            ~Window() {
                glfwDestroyWindow(window);
                glfwTerminate();
            }

            GLFWwindow* getWindow() {
                return window;
            }

            static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
                auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
                w->framebufferResized = true;
                w->WIDTH = width;
                w->HEIGHT = height;
            }

            VkExtent2D GetExtent() {
                return {static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT)};
            }

            bool WasWindowResized() { return framebufferResized; }
            void ResetWindowResizedFlag() { framebufferResized = false; }
        private:

        int WIDTH = 1900;
        int HEIGHT = 1000;

        GLFWwindow* window;

        bool framebufferResized;
    }; 
}