#include "Window.h"

#include "GLFW/glfw3.h"
#include "Logger.h"
#include "VulkanHelpUtils.h"

#include <cassert>
namespace sge {
Window::Window(int width, int height, const std::string& name) noexcept
    : m_width(width), m_height(height), m_windowName(name) {
    init();
}

Window::~Window() {
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

bool Window::shouldClose() noexcept { return glfwWindowShouldClose(m_pWindow); }

bool Window::framebufferResized() const noexcept { return m_framebufferResized; }

void Window::resetWindowResizeFlag() noexcept { m_framebufferResized = false; }

VkExtent2D Window::getExtent() const noexcept {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) noexcept {
    auto result = glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface);
    VK_CHECK_RESULT(result, "Failed to create window surface!")
}

void Window::closeWindow() noexcept { glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE); }
/*static*/ void Window::frameResizeCallback(GLFWwindow* pWindow, int width, int height) noexcept {
    auto resizedWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
    resizedWindow->m_framebufferResized = true;
    resizedWindow->m_width = width;
    resizedWindow->m_height = height;
}

void Window::init() noexcept {
    if (int initGLFW = glfwInit(); initGLFW != GLFW_TRUE) {
        LOG_ERROR("Can't init GLFW!")
        assert(false);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_pWindow = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
    if (m_pWindow == nullptr) {
        LOG_ERROR("Can't create GLFW window!")
        assert(false);
    }

    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, frameResizeCallback);

    glfwSetKeyCallback(m_pWindow, [](GLFWwindow* pWindow, int key, int scancode, int action, int mods) {
        Window& data = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));

        EventKeyPressed event(key, scancode, action, mods);
        data.m_eventCallback(event);
    });

    glfwSetScrollCallback(m_pWindow, [](GLFWwindow* pWindow, double x, double y) {
        Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(pWindow));

        EventScroll event(x, y);
        data.m_eventCallback(event);
    });

    glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* pWindow, int width, int height) {
        Window& data = *static_cast<Window*>(glfwGetWindowUserPointer(pWindow));
        data.m_width = width;
        data.m_height = height;
        EventWindowResize event(width, height);
        data.m_eventCallback(event);
    });
}
}  // namespace sge
