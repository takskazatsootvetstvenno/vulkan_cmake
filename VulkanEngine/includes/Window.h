#pragma once
#include "Event.h"

#include <vulkan/vulkan_core.h>

#include <string>
struct GLFWwindow;
namespace sge {

class Window {
 public:
    using EventCallbackFn = std::function<void(BaseEvent&)>;
    Window(int width, int height, const std::string& name) noexcept;
    ~Window();
    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    bool shouldClose() noexcept;
    bool framebufferResized() const noexcept;
    void resetWindowResizeFlag() noexcept;
    VkExtent2D getExtent() const noexcept;
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) noexcept;
    void closeWindow() noexcept;
    void set_event_callback(const EventCallbackFn& callback) { m_eventCallback = callback; }
    GLFWwindow* getHandle() { return m_pWindow; }

 private:
    static void frameResizeCallback(GLFWwindow* window, int width, int height) noexcept;
    void init() noexcept;

    int m_width;
    int m_height;
    GLFWwindow* m_pWindow = nullptr;
    std::string m_windowName;
    bool m_framebufferResized = false;
    EventCallbackFn m_eventCallback;
};
}  // namespace sge
