#include "teng_window.hpp"
#include <iostream>
#include <stdexcept>
#include <chrono>

namespace teng {

  Window::Window(int h, int w, std::string name)
    : m_Width{w}, m_Height{h}, m_WindowName{name} {
    m_InitWindow();
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  // Destructor
  Window::~Window() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
  }

  void ErrorCallback(int, const char *err_str) {
    std::cout << "GLFW Error: " << err_str << std::endl;
  }

  void Window::m_InitWindow() {
    glfwInit();
    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API); // GLFW_NO_API implies that we're not using
                                 // OpenGL (but Vulcan)
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);


    m_Window = glfwCreateWindow(
      m_Width,
      m_Height,
      m_WindowName.c_str(),
      nullptr,
      nullptr);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, m_FramebufferResizeCallback);
    glfwSetCursorPosCallback(m_Window, m_MouseMoveCallback);
  }

  void Window::createWindowSurface(VkInstance instance,
                                       VkSurfaceKHR *surface) {
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface");
    }
  }

  void Window::m_FramebufferResizeCallback(GLFWwindow *window, int newWidth, int newHeight) {
    auto p_TengWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    p_TengWindow->m_IsFramebufferResized = true;
    p_TengWindow->m_Width = newWidth;
    p_TengWindow->m_Height = newHeight;
  };

  void Window::m_MouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
    auto p_TengWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    p_TengWindow->mousePosition = glm::vec2(xpos, ypos);
  };

  } // namespace teng
