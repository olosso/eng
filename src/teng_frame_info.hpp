#pragma once

#include "camera.hpp"

#include <vulkan/vulkan.h>

namespace teng {

    struct FrameInfo {
        int             backFrame;
        float           frameTime;
        VkCommandBuffer commandBuffer;
        Camera&         camera;
        VkDescriptorSet globalDescriptorSet;
    };
}
