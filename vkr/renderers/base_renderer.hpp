#pragma once

#include "../../ecs/ecs.hpp"

// std
#include <memory>

namespace vkr {
    class Renderer {
        public:
        Renderer(std::shared_ptr<Device> d, std::shared_ptr<Swapchain> s, VkPrimitiveTopology topology, std::string vertpath, std::string fragpath, uint32_t tI = 0) : device{d}, swapchain{s}, TOPOLOGY{topology}, VERT_PATH{vertpath}, FRAG_PATH{fragpath}, typeIndex{tI} {

        }
        virtual ~Renderer() = default;

        virtual std::vector<std::shared_ptr<ecs::Entity>>& GetEntities() {
            return entities;
        }

        virtual void Render(VkCommandBuffer commandBuffer) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

            for (auto& entity : entities) {
                if (entity->HasComponent<ecs::Mesh2D>()) {
                    auto buffer = entity->GetComponent<ecs::Mesh2D>().GetMesh()->GetVertexBuffer();

                    VkBuffer buffers[] = {buffer->GetBuffer()};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                    vkCmdDraw(commandBuffer, buffer->GetInstanceCount(), 1, 0, 0);
                }
                else if (entity->HasComponent<ecs::Mesh3D>()) {
                    auto buffer = entity->GetComponent<ecs::Mesh3D>().GetMesh()->GetVertexBuffer();

                    VkBuffer buffers[] = {buffer->GetBuffer()};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
                    vkCmdDraw(commandBuffer, buffer->GetInstanceCount(), 1, 0, 0);
                }
            }
        }

        protected:
            std::shared_ptr<Device> device;
            std::shared_ptr<Swapchain> swapchain;

            std::vector<std::shared_ptr<ecs::Entity>> entities;

            std::string VERT_PATH;
            std::string FRAG_PATH;
            VkPrimitiveTopology TOPOLOGY;
            uint32_t typeIndex;

            Pipeline pipeline{*device, *swapchain, TOPOLOGY, VERT_PATH, FRAG_PATH, typeIndex};
    };
    class TriangleRenderer2D : public Renderer {
        public:
            TriangleRenderer2D(std::shared_ptr<Device> d, std::shared_ptr<Swapchain> s) : Renderer{d, s, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, "../vkr/renderers/shaders/SPIR-V/base_triangle_2d.vert.spv", "../vkr/renderers/shaders/SPIR-V/base_triangle_2d.frag.spv", 1} {
            }
            ~TriangleRenderer2D() {}
    };
    class TriangleRenderer3D : public Renderer {
        public:
            TriangleRenderer3D(std::shared_ptr<Device> d, std::shared_ptr<Swapchain> s) : Renderer{d, s, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, "../vkr/renderers/shaders/SPIR-V/base_triangle_3d.vert.spv", "../vkr/renderers/shaders/SPIR-V/base_triangle_3d.frag.spv"} {
            
            }
            ~TriangleRenderer3D() {}
    };
    class LineRenderer2D : public Renderer {
        protected:
    };
    class LineRenderer3D : public Renderer {
        protected:
    };
}