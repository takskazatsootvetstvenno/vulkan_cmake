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
    KeyInfo& Window::getLastKeyInfo() noexcept
    {
        return m_lastKey;
    }
    void Window::closeWindow() noexcept
    {
        glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
    }
    /*static*/ void Window::frameResizeCallback(GLFWwindow* pWindow, int width, int height) noexcept
    {
        auto resizedWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
        resizedWindow->m_framebufferResized = true;
        resizedWindow->m_width = width;
        resizedWindow->m_height = height;
    }
    /*static*/ void Window::keyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods) noexcept
    {
        auto windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
        windowPtr->m_lastKey.key = key;
        windowPtr->m_lastKey.scancode = scancode;
        windowPtr->m_lastKey.action = action;
        windowPtr->m_lastKey.mods = mods;
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
            case GLFW_KEY_Q:
                windowPtr->m_lastKey.key_Q_pressed = true;
                windowPtr->m_lastKey.key_E_pressed = false;
                break;
            case GLFW_KEY_E:
                windowPtr->m_lastKey.key_E_pressed = true;
                windowPtr->m_lastKey.key_Q_pressed = false;
                break;
            case GLFW_KEY_W:
                windowPtr->m_lastKey.key_W_pressed = true;
                windowPtr->m_lastKey.key_S_pressed = false;
                break;
            case GLFW_KEY_S:
                windowPtr->m_lastKey.key_S_pressed = true;
                windowPtr->m_lastKey.key_W_pressed = false;
                break;
            case GLFW_KEY_SPACE:
                windowPtr->m_lastKey.key_space_pressed = true;
                windowPtr->m_lastKey.key_E_pressed = false;
                windowPtr->m_lastKey.key_Q_pressed = false;
                windowPtr->m_lastKey.key_W_pressed = false;
                windowPtr->m_lastKey.key_S_pressed = false;
                break;
            case GLFW_KEY_ESCAPE:
                windowPtr->closeWindow();
                break;
            default:
                windowPtr->m_lastKey.key_E_pressed = false;
                windowPtr->m_lastKey.key_Q_pressed = false;
                windowPtr->m_lastKey.key_W_pressed = false;
                windowPtr->m_lastKey.key_S_pressed = false;
            }
        }

    }
    void Window::scrollCallback(GLFWwindow* pWindow, double xoffset, double yoffset) noexcept
    {
        auto windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
        if (yoffset > windowPtr->m_lastKey.scroll)
        {
            windowPtr->m_lastKey.scroll_direction = -5;
            windowPtr->m_lastKey.scroll = yoffset;
        }
        else
        {
            windowPtr->m_lastKey.scroll_direction = 5;
            windowPtr->m_lastKey.scroll = yoffset;
        }
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
        glfwSetKeyCallback(m_pWindow, keyCallback);
        glfwSetScrollCallback(m_pWindow, scrollCallback);
        if (m_pWindow == nullptr)
        {
            LOG_ERROR("Can't create GLFW window!")
            assert(false);
        }
    }
}  // namespace sge