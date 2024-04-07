#pragma once

#include "null_component.hpp"

#include <qbn.hpp>

namespace ecs {
    class Transform2D : public Component {
        private:
            
        public:
        qbn::vec<float, 2> position{0, 0};
        qbn::vec<float, 2> scale{1, 1};
        float rotation{0};
    };

    class Transform3D : public Component {
        private:
            
        public:
        qbn::vec<float, 3> position{0, 0, 0};
        qbn::vec<float, 3> scale{1, 1, 1};
        qbn::vec<float, 3> rotation{0, 0, 0};
    };
}