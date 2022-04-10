#pragma once
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
#include <memory>
namespace sge {
	class Renderer {
	public:
		Renderer(Window& window, Device& device);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkCommandBuffer beginFrame() noexcept;
		VkCommandBuffer getCurrentCommandBuffer() const noexcept;
		VkRenderPass getSwapChainRenderPass() const noexcept;
		bool isFrameInProgress() const;
		bool endFrame() noexcept;
		uint32_t getCurrentImageIndex() const noexcept;
		int getFrameIndex() const noexcept;
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept;
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept;

	private:
		void freeCommandBuffers();
		void createCommandBuffers();
		void recreateSwapChain() noexcept;

		Window& m_window;
		Device& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t m_currentImageIndex;
		bool m_isFrameStarted = false;
		int m_currentFrameIndex = 0;
	};

}