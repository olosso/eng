#pragma once

#include "teng_device.hpp"
#include <string>
#include <vector>

namespace teng {

struct PipelineConfigInfo {

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class Pipeline {
public:
    Pipeline(Device &device, const std::string &vertFilepath,
               const std::string &fragFilepath,
               const PipelineConfigInfo &config);

    ~Pipeline();

    // Delete copy constructor, so that we don't accidentally copy
    // Vulkan device pointers.
    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;

    void bind(VkCommandBuffer commandBuffer);

    static void s_DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

private:
  static std::vector<char> m_ReadFile(const std::string &filePath);
  void m_CreateGraphicsPipeline(const std::string &vertFilepath,
                                const std::string &fragFilepath,
                                const PipelineConfigInfo &config);

  void m_CreateShaderModule(const std::vector<char> &code,
                            VkShaderModule *shaderModule);

  Device &m_Device;
  VkPipeline m_VkPipeline;         // NOTE VkPipeline contains an address
  VkShaderModule m_VkShaderModule; // NOTE VkShaderModule contains an address
  VkShaderModule m_VkFragModule;   // NOTE VkShaderModule contains an address
};
} // namespace teng
