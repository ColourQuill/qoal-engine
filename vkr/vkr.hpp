#pragma once

#include "rendering/rendering.hpp"
#include "renderers/renderers.hpp"

// std
#include <cstring>
#include <memory>
#include <mutex>

namespace vkr {
    class VulkanRendering {
        public:
            ~VulkanRendering() {
                Clean();
            }

            void Init() {
                window = std::make_shared<Window>();
                validationLayers = std::make_shared<ValidationLayers>();
                instance = std::make_shared<Instance>(*validationLayers);
                debugger = std::make_shared<Debugger>(*validationLayers, *instance);
                surface = std::make_shared<Surface>(*window, *instance);
                device = std::make_shared<Device>(*window, *validationLayers, *instance, *surface);
                swapchain = std::make_shared<Swapchain>(*window, *surface, *device);
                command_pool = std::make_shared<CommandPool>(*device, *swapchain);
                render = std::make_shared<Render>(*swapchain, *command_pool);
                bufferManager = std::make_shared<BufferManager>(*device, *command_pool);
                mesh_pool = std::make_shared<MeshPool>(bufferManager);
            }

            void Run() {
                glfwPollEvents();
                auto commandBuffer = render->BeginFrame();
                render->BeginSwapchainRenderpass(commandBuffer);
                for (auto& renderer : renderers) {
                    renderer->Render(commandBuffer);
                }
                render->EndSwapchainRenderpass(commandBuffer);
                render->EndFrame();

                vkDeviceWaitIdle(device->GetDevice());
            }

            void Clean() {
                render.reset();
                for (auto& buffer : bufferManager->GetBufferPool()) {
                    buffer->~Buffer();
                }
                for (auto& renderer : renderers) {
                    renderer.reset();
                }
                renderers.clear();
                command_pool.reset();
                swapchain.reset();
                device.reset();
                debugger.reset();
                surface.reset();
                instance.reset();
                validationLayers.reset();
                window.reset();
            }

            void SetRendererEntities(std::unordered_map<std::shared_ptr<vkr::Renderer>, std::vector<std::shared_ptr<ecs::Entity>>>& RendererEntities) {
                for (auto& pair : RendererEntities) {
                    bool found = false;
                    for (auto& renderer : renderers) {
                        if (auto derivedRenderer = std::dynamic_pointer_cast<decltype(pair.first)>(renderer)) {
                            if (typeid(*derivedRenderer) == typeid(*pair.first)) {
                                // Renderer found, do something
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found) {
                        // Renderer not found, do something else
                        auto renderer = pair.first;
                        renderer.get()->GetEntities() = pair.second;
                        renderers.push_back(renderer);
                    }
                }
            }

            Window& GetWindow() {
                return *window;
            }

            std::shared_ptr<Device> GetDevice() {
                return device;
            }

            std::shared_ptr<Swapchain> GetSwapchain() {
                return swapchain;
            }

            std::shared_ptr<BufferManager> GetBufferManager() {
                return bufferManager;
            }

            std::shared_ptr<MeshPool> GetMeshPool() {
                return mesh_pool;
            }

            std::mutex renderersMutex;
        private:
            std::shared_ptr<Window> window;
            std::shared_ptr<ValidationLayers> validationLayers;
            std::shared_ptr<Instance> instance;
            std::shared_ptr<Debugger> debugger;
            std::shared_ptr<Surface> surface;
            std::shared_ptr<Device> device;
            std::shared_ptr<Swapchain> swapchain;
            std::shared_ptr<CommandPool> command_pool;
            std::shared_ptr<Render> render;

            std::vector<std::shared_ptr<vkr::Renderer>> renderers;
            std::shared_ptr<BufferManager> bufferManager;
            std::shared_ptr<MeshPool> mesh_pool;
    };
}