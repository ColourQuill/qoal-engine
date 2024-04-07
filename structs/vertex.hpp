#pragma once

#include "GLFW/glfw3.h"

#include <qbn.hpp>

// std
#include <vector>

struct Vertex2D {
    qbn::vec<float, 2> position;
    qbn::vec<float, 4> colour;
    qbn::vec<float, 2> tex;

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescributions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex2D, colour)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, tex)});

        return attributeDescriptions;
    }

    static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex2D);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
};

struct Vertex3D {
    qbn::vec<float, 3> position;
    qbn::vec<float, 3> normal;
    qbn::vec<float, 4> colour;
    qbn::vec<float, 2> tex;

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescributions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, normal)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex3D, colour)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, tex)});

        return attributeDescriptions;
    }

    static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex3D);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
};