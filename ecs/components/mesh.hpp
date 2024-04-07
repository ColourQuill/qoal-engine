#pragma once

#include "null_component.hpp"
#include "../../structs/vertex.hpp"

// std
#include <vector>

// qbn
#include <qbn.hpp>

// debug
#include <iostream>

namespace ecs {
    class Mesh2D : public Component {
        private:
            std::shared_ptr<vkr::Mesh> mesh;
        public:
            Mesh2D(std::shared_ptr<vkr::Mesh> m) : mesh{m} {}

            std::shared_ptr<vkr::Mesh> GetMesh() {
                return mesh;
            }
    };

    class Mesh3D : public Component {
        private:
            std::shared_ptr<vkr::Mesh> mesh;
        public:
            Mesh3D(std::shared_ptr<vkr::Mesh> m) : mesh{m} {}

            std::shared_ptr<vkr::Mesh> GetMesh() {
                return mesh;
            }
    };
}