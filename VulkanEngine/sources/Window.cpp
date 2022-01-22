#include "Window.h"
#include "GLFW/glfw3.h"
#include <cassert>
namespace sge {
    Window::Window(int width, int height, std::string name)
        : m_width(width), m_height(height), m_windowName(name) {
        init();
    }
    Window::~Window() { 
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }
    bool Window::shouldClose() { 
        return glfwWindowShouldClose(m_pWindow); 
    }
    VkExtent2D Window::getExtent() {
        return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
    }
    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) 
    {
        auto result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface);
        assert(result == VK_SUCCESS && "failed to create window surface");
    }
    void Window::init() { 
        int initGLFW = glfwInit();
        assert(GLFW_TRUE == initGLFW && "Can't init GLFW!");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_pWindow = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
        assert(nullptr != m_pWindow && "Can't create GLFW window!");
    }
}  // namespace sge