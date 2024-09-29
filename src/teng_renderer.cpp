#include "teng_renderer.hpp"

#include "app.hpp"
#include <iostream>
#include <stdexcept>

#include <chrono>
#include <array>


namespace teng {

    // Public
    Renderer::Renderer(Window& window, Device& device)
        : mr_Window(window), mr_Device(device)
    {
        m_RecreateSwapChain();
        m_CreateCommandBuffers();
    }

    Renderer::~Renderer() {
        // Renderer is responsible for command buffers.
        m_FreeCommandBuffers();
    };

    void Renderer::m_CreateCommandBuffers() {
        mp_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = mr_Device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(mp_CommandBuffers.size());

        if(vkAllocateCommandBuffers(mr_Device.device(), &allocInfo, mp_CommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffer");
        };

    };

    void Renderer::m_FreeCommandBuffers() {

        vkFreeCommandBuffers(
            mr_Device.device(),
            mr_Device.getCommandPool(),
            static_cast<uint32_t>(mp_CommandBuffers.size()),
            mp_CommandBuffers.data());

        mp_CommandBuffers.clear();
    }

    void Renderer::m_RecreateSwapChain() {
        // Current window size.
        auto extent = mr_Window.getExtent();

        // Force program to halt while minimized for instance.
        while(extent.height == 0 || extent.width == 0) {
            extent = mr_Window.getExtent();
            glfwWaitEvents();
        };

        // Wait for GPU finish its queue. Then the swap chain can be recreated and a new image drawn.
        vkDeviceWaitIdle(mr_Device.device());

        // Create new swap chain. The unique_ptr should ensure that previous swap chain is dropped.
        // mp_SwapChain = nullptr;
        if(mp_SwapChain == nullptr) {
            mp_SwapChain = std::make_unique<SwapChain>(mr_Device, extent);
        } else {

            std::shared_ptr<SwapChain> oldSwapChain = std::move(mp_SwapChain);

            mp_SwapChain = std::make_unique<SwapChain>(
                mr_Device,
                extent,
                oldSwapChain); // The old swap chain is moved so that the pipeline can decide to use it if can.

            if(!mp_SwapChain->compareSwapFormats(*mp_SwapChain.get())) {
                throw std::runtime_error("m_RecreateSwapChain: incompatible swap chain formats");
            }
        }
    };

    VkCommandBuffer Renderer::beginFrame() {

        assert(!m_IsFrameStarted && "can call beginFrame only when a frame isn't already in progress");
        static uint32_t frames = 0;
        static auto previous = std::chrono::steady_clock::now();
        const auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now - previous).count() >= 5000) {
            std::cout << "ms per frame: " << 5000/static_cast<double_t>(frames) << "\n";
            previous = now;
            frames = 0;
        };
        frames = frames + 1;

        // The swap chain knows where the data for the next image is to be stored in.
        auto result = mp_SwapChain->acquireNextImage(&m_CurrentImageIndex);

        // Here we can check whether the surface has changed and is now incompatible with current swap chain dimensions.
        // If so we must recreate the swapchain.
        if(result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_RecreateSwapChain();
            return nullptr;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("beginFrame: failed to obtain next swap chain image");
        }

        m_IsFrameStarted = true;
        auto commandBuffer = p_GetCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        return commandBuffer;
    };

    void Renderer::endFrame() {
        assert(m_IsFrameStarted && "can call endFrame only when there is a frame to end");

        auto commandBuffer = p_GetCurrentCommandBuffer();
        if(vkEndCommandBuffer(p_GetCurrentCommandBuffer()) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }

        auto result = mp_SwapChain->submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

        // Not sure why this is necessary.
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mr_Window.wasFrameBufferResized()) {
            mr_Window.resetFramebufferResized();
            m_RecreateSwapChain();
        } else if(result != VK_SUCCESS) {
            throw std::runtime_error("endFrame: failed to obtain next swap chain image");
        }

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
        m_IsFrameStarted = false;
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer p_CommandBuffer) {
        assert(m_IsFrameStarted && "can't call beginSwapChainRenderPass if frame is not in progress");
        assert(p_CommandBuffer == p_GetCurrentCommandBuffer() && "can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mp_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = mp_SwapChain->getFrameBuffer(m_CurrentImageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mp_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
        // This was set to 0.1f instead of 1.0f, the depth of the volume to be cut to a tenth of what it should have been.
        clearValues[1].depthStencil = {1.f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(p_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mp_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(mp_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, mp_SwapChain->getSwapChainExtent()};
        vkCmdSetViewport(p_CommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(p_CommandBuffer, 0, 1, &scissor);
    };

    void Renderer::endSwapChainRenderPass(VkCommandBuffer p_CommandBuffer) {
        assert(m_IsFrameStarted && "can't call endSwapChainRenderPass if frame is not in progress");
        assert(p_CommandBuffer == p_GetCurrentCommandBuffer() && "can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(p_CommandBuffer);
    };
} // namespace teng
