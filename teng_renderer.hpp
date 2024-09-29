#pragma once

#include "teng_swap_chain.hpp"
#include "teng_window.hpp"
#include "teng_model.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace teng {

        class Renderer {

                public:

                        Renderer(Window& window, Device& device);
                        ~Renderer();

                        Renderer(const Renderer&) = delete;
                        Renderer &operator=(const Renderer&) = delete;

                        void run();

                        VkCommandBuffer beginFrame();
                        void endFrame();
                        void beginSwapChainRenderPass(VkCommandBuffer p_CommandBuffer);
                        void endSwapChainRenderPass(VkCommandBuffer p_CommandBuffer);
                        float getAspectRatio() { return mp_SwapChain->extentAspectRatio(); };

                        VkRenderPass p_GetSwapChainRenderPass() const { return mp_SwapChain->getRenderPass(); };
                        bool isFrameInProgress() const { return m_IsFrameStarted; };
                        VkCommandBuffer p_GetCurrentCommandBuffer() const {
                                assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress.");
                                return mp_CommandBuffers[m_CurrentFrameIndex];
                        };

                        int getCurrentFrameIndex() const {
                                assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress.");
                                return m_CurrentFrameIndex;
                        };

                private:

                        void m_CreateCommandBuffers();
                        void m_FreeCommandBuffers();
                        void m_RecreateSwapChain(); // The swapchain needs to be recreated for example when the window is resized.

                        Window& mr_Window; // Window must outlive renderer!
                        Device& mr_Device; // Device must outlive renderer!
                        std::unique_ptr<SwapChain> mp_SwapChain;
                        std::vector<VkCommandBuffer> mp_CommandBuffers;

                        uint32_t m_CurrentImageIndex{0};
                        int m_CurrentFrameIndex{0};
                        bool m_IsFrameStarted{false};

        };
} // namespace teng
