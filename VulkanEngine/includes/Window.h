#pragma once
#include <string>
#include "volk.h"
struct GLFWwindow;
namespace sge {
    struct KeyInfo
    {
        int key = 0;
        int scancode = 0;
        int action = 0;
        int mods = 0;
        float scroll = 0.f;
        int scroll_direction = 0;
        bool key_Q_pressed = false;
        bool key_E_pressed = false;
        bool key_W_pressed = false;
        bool key_S_pressed = false;
        bool key_space_pressed = false;
    };
    class Window {
    public:
        Window(int width, int height, std::string name) noexcept;
        ~Window();
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        bool shouldClose() noexcept;
        bool framebufferResized() const noexcept ;
        void resetWindowResizeFlag() noexcept;
        VkExtent2D getExtent() const noexcept;
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) noexcept;
        KeyInfo& getLastKeyInfo() noexcept;
        void closeWindow() noexcept;
    private:
        static void frameResizeCallback(GLFWwindow* window, int width, int height) noexcept;
        static void keyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods) noexcept;
        static void scrollCallback(GLFWwindow* pWindow, double xoffset, double yoffset) noexcept;
        void init() noexcept;

        int m_width;
        int m_height;
        GLFWwindow* m_pWindow = nullptr;
        std::string m_windowName;
        bool m_framebufferResized = false;
        KeyInfo m_lastKey;
    };
}