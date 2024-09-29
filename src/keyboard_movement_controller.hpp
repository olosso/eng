#pragma once

#include "teng_game_object.hpp"
#include "teng_window.hpp"

namespace teng {
    class KeyboardMovementController {
        public:
            struct KeyMappings {
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_E;
                int moveDown = GLFW_KEY_Q;

                int lookLeft = GLFW_KEY_J;
                int lookRight = GLFW_KEY_L;
                int lookUp = GLFW_KEY_I;
                int lookDown = GLFW_KEY_K;
            };

            void moveInPlaneXZ(GLFWwindow* a_Window, float dt, GameObject& r_GameObject);

            void rotateWithMouse(GLFWwindow* a_Window, float dt, GameObject& r_GameObject, glm::vec2 mouseDelta);

            KeyMappings keys{};
            float moveSpeed{3.f};
            float turnSpeed{1.5f};
            float mouseSensitivity{2.f};
            float mouseDeadzone{0.3f};
    };
}
