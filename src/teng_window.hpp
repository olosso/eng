#pragma once

#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN // Include Vulkan headers associated with GLFW
#include <GLFW/glfw3.h>
#include <string>

namespace teng {

        class Window {

                public:

                        Window(int w, int h, std::string name);
                        ~Window();

                        bool shouldClose() { return glfwWindowShouldClose(m_Window); }
                        bool wasFrameBufferResized() { return m_IsFramebufferResized; }
                        void resetFramebufferResized() { m_IsFramebufferResized = false; }

                        VkExtent2D getExtent() { return VkExtent2D{static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height)}; };

                        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
                        GLFWwindow* getWindow() {
                                return m_Window;
                        }

                        glm::vec2 getMousePosition() { return mousePosition; };


                private:

                        // Functions
                        static void m_FramebufferResizeCallback(GLFWwindow *window, int width, int height);
                        static void m_MouseMoveCallback(GLFWwindow *window, double xpos, double ypos);
                        void m_InitWindow();

                        // Variables
                        int m_Width;
                        int m_Height;
                        bool m_IsFramebufferResized = false;
                        glm::vec2 mousePosition;
                        std::string m_WindowName;
                        GLFWwindow *m_Window;
};

} // namespace teng
