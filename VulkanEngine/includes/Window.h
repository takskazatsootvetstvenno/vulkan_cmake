#pragma once
#include <string>
#include <vulkan/vulkan.h>
struct GLFWwindow;
namespace sge {
    class Window {
    public:
        Window(int width, int height, std::string name);
        ~Window();
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        bool shouldClose();
        VkExtent2D getExtent();
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    private:
        void init();
        const int m_width;
        const int m_height;
        std::string m_windowName;
        GLFWwindow* m_pWindow = nullptr;
    };
}  // namespace sge