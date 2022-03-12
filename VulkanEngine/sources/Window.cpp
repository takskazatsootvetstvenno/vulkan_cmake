#include "Window.h"
#include "GLFW/glfw3.h"
#include "Logger.h"
#include <cassert>
namespace sge {
    Window::Window(int width, int height, std::string name) noexcept
        : m_width(width), m_height(height), m_windowName(name) {
        init();
    }
    Window::~Window() { 
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }
    bool Window::shouldClose() noexcept
    {
        return glfwWindowShouldClose(m_pWindow); 
    }
    bool Window::framebufferResized() const noexcept
    {
        return m_framebufferResized;
    }
    void Window::resetWindowResizeFlag() noexcept
    {
        m_framebufferResized = false;
    }
     VkExtent2D Window::getExtent() const noexcept
    {
        return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
    }
    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) noexcept
    {
        auto result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create window surface!")
            assert(false);
        }
    }
    /*static*/ void Window::frameResizeCallback(GLFWwindow* window, int width, int height) noexcept
    {
        auto resizedWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        resizedWindow->m_framebufferResized = true;
        resizedWindow->m_width = width;
        resizedWindow->m_height = height;
    }
    void Window::init() noexcept {
        int initGLFW = glfwInit();
        if (initGLFW != GLFW_TRUE)
        {
            LOG_ERROR("Can't init GLFW!")
            assert(false);
        }        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_pWindow = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_pWindow, this);
        glfwSetFramebufferSizeCallback(m_pWindow, frameResizeCallback);
        if (m_pWindow == nullptr)
        {
            LOG_ERROR("Can't create GLFW window!")
            assert(false);
        }
    }
}  // namespace sge