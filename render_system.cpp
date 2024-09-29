#include "render_system.hpp"

#include <iostream>
#include <stdexcept>

#include <chrono>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace teng {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    // Public
    RenderSystem::RenderSystem(Device& r_Device, VkRenderPass a_RenderPass, VkDescriptorSetLayout globalSetLayout)
        : mr_Device{r_Device}
    {
        m_CreatePipelineLayout(globalSetLayout);
        m_CreatePipeline(a_RenderPass);
    }

    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(mr_Device.device(), mp_PipelineLayout, nullptr);
    };

    void RenderSystem::m_CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout) {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayout{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if(vkCreatePipelineLayout(mr_Device.device(), &pipelineLayoutInfo, nullptr, &mp_PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        };
    };


    void RenderSystem::m_CreatePipeline(VkRenderPass a_RenderPass) {
        assert(mp_PipelineLayout != nullptr && "swap chain not initialized");

        PipelineConfigInfo pipelineInfo{};
        Pipeline::s_DefaultPipelineConfigInfo(pipelineInfo);
        pipelineInfo.renderPass = a_RenderPass;
        pipelineInfo.pipelineLayout = mp_PipelineLayout;
        m_Pipeline = std::make_unique<Pipeline>(
            mr_Device,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineInfo);
    };

    void RenderSystem::m_RenderGameObjects(FrameInfo& frameInfo, std::vector<GameObject>& gameObjects) {
        m_Pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mp_PipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto& obj : gameObjects) {

            SimplePushConstantData push{};
            push.modelMatrix = obj.p_Transform.mat4();
            push.normalMatrix = obj.p_Transform.normalMatrix();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                mp_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            if(!obj.model) {
                throw std::runtime_error("Tried to render a GameObject without a model.");
            };

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    // Angle between two 2D vectors.
    float RenderSystem::angle(const glm::vec2& a, glm::vec2& b) {
        return glm::acos(glm::dot<2,float>(a, b)/(glm::length(a)*glm::length(b)));
    };

} // namespace teng
