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
		void run();

		VkCommandBuffer beginFrame();
		bool endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkRenderPass getSwapChainRenderPass() const;
		bool isFrameInProgress() const;
		VkCommandBuffer getCurrentCommandBuffer() const;
	private:
		void freeCommandBuffers();
		void createCommandBuffers();
		void recreateSwapChain() noexcept;
		//void recordCommandBuffer(const int imageIndex);

		Window& m_window;
		Device& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;
		uint32_t m_currentImageIndex;
		bool m_isFrameStarted = false;
	};

}