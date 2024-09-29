#include "keyboard_movement_controller.hpp"
#include <iostream>
#include <limits>

namespace teng {

    void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* a_Window, float dt, GameObject& r_GameObject) {

        glm::vec3 rotate{0};
        if(glfwGetKey(a_Window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
        if(glfwGetKey(a_Window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if(glfwGetKey(a_Window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
        if(glfwGetKey(a_Window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

        if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            r_GameObject.p_Transform.rotation += turnSpeed * dt * glm::normalize(rotate);
        };
        r_GameObject.p_Transform.rotation.x = glm::clamp(r_GameObject.p_Transform.rotation.x, -1.5f, 1.5f);
        r_GameObject.p_Transform.rotation.y = glm::mod(r_GameObject.p_Transform.rotation.y, glm::two_pi<float>());

        float yaw = r_GameObject.p_Transform.rotation.y;
        const glm::vec3 forward{glm::sin(yaw), 0.f, glm::cos(yaw)};
        const glm::vec3 right{forward.z, 0.f, -forward.x};
        const glm::vec3 up{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if(glfwGetKey(a_Window, keys.moveForward) == GLFW_PRESS) moveDir += forward;
        if(glfwGetKey(a_Window, keys.moveBackward) == GLFW_PRESS) moveDir -= forward;
        if(glfwGetKey(a_Window, keys.moveRight) == GLFW_PRESS) moveDir += right;
        if(glfwGetKey(a_Window, keys.moveLeft) == GLFW_PRESS) moveDir -= right;
        if(glfwGetKey(a_Window, keys.moveUp) == GLFW_PRESS) moveDir += up;
        if(glfwGetKey(a_Window, keys.moveDown) == GLFW_PRESS) moveDir -= up;

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            r_GameObject.p_Transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        };

    };

    void KeyboardMovementController::rotateWithMouse(GLFWwindow* a_Window, float dt, GameObject& r_GameObject, glm::vec2 mouseDelta) {

        glm::vec3 rotate{0};
        rotate.y = mouseDelta.x;
        rotate.x = -mouseDelta.y;

        if(glm::length(rotate) > mouseDeadzone) {
            r_GameObject.p_Transform.rotation += mouseSensitivity * dt * glm::normalize(rotate);
        };

        // r_GameObject.p_Transform.rotation.x = glm::clamp(r_GameObject.p_Transform.rotation.x, -1.5f, 1.5f);
        // r_GameObject.p_Transform.rotation.y = glm::mod(r_GameObject.p_Transform.rotation.y, glm::two_pi<float>());
    }

}
