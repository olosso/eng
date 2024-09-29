#pragma once

#include "teng_pipeline.hpp"
#include "teng_model.hpp"
#include "teng_game_object.hpp"
#include "teng_frame_info.hpp"
#include "camera.hpp"

// std
#include <memory>
#include <vector>

namespace teng {

    class RenderSystem {

        public:

            RenderSystem(Device& r_Device, VkRenderPass p_RenderPass, VkDescriptorSetLayout globalSetLayout);
            ~RenderSystem();

            RenderSystem(const RenderSystem&) = delete;
            RenderSystem &operator=(const RenderSystem&) = delete;
            void m_RenderGameObjects(
                FrameInfo& frameInfo,
                std::vector<GameObject> &r_GameObjects);
            void m_RenderGrav(VkCommandBuffer p_CommandBuffer, std::vector<GameObject> &r_GameObjects);

        private:

            void m_CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void m_CreatePipeline(VkRenderPass p_RenderPass);
            float angle(const glm::vec2& a, glm::vec2& b);

            Device &mr_Device; // Creates a device when RenderSystem is constructed.
            std::unique_ptr<Pipeline> m_Pipeline;
            VkPipelineLayout mp_PipelineLayout;
};
} // namespace teng
