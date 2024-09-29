#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace teng {

    class Camera {

        public:
            void setOrthographicProjection(
                float left,
                float right,
                float bottom,
                float top,
                float near,
                float far);

            void setPerspectiveProjection(
                float fov,
                float aspectRatio,
                float near,
                float far);


            void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
            void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

            // Const reference so that the matrix isn't modified.
            const glm::mat4& getProjectionMatrix() const {
                return m_ProjectionMatrix;
            };

            const glm::mat4& getViewMatrix() const {
                return m_ViewMatrix;
            };

        private:
            glm::mat4 m_ProjectionMatrix{1.f};
            glm::mat4 m_ViewMatrix{1.f};
    };
}
