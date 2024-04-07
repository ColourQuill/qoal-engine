#pragma once

namespace ecs {
    class Camera : public Component {
        public:
            void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
                projectionMatrix = qbn::mat<float, 4>{1};
                projectionMatrix[0][0] = 2.0f / (right - left);
                projectionMatrix[1][1] = 2.0f / (bottom - top);
                projectionMatrix[2][2] = 1.0f / (far - near);
                projectionMatrix[3][0] = -(right + left) / (right - left);
                projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
                projectionMatrix[3][2] = -near / (far - near);
            }

            void SetPerspectiveProjection(float fovy, float aspect, float near, float far) {
                const float tanHalfFovy = tan(fovy / 2.0f);
                projectionMatrix = qbn::mat<float, 4>{0};
                projectionMatrix[0][0] = 1.0f / (aspect * tanHalfFovy);
                projectionMatrix[1][1] = 1.0f / (tanHalfFovy);
                projectionMatrix[2][2] = far / (far - near);
                projectionMatrix[2][3] = 1.0f;
                projectionMatrix[3][2] = -(far * near) / (far - near);
            }

            const qbn::mat<float, 4>& GetProjection() const { return projectionMatrix; }
        private:
            qbn::mat<float, 4> projectionMatrix;
    };
}