#include "app.hpp"
#include <iostream>
#include <stdexcept>

#include "render_system.hpp"
#include "teng_buffer.hpp"
#include "keyboard_movement_controller.hpp"
#include "camera.hpp"
#include <chrono>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace teng {

    // Global data for shaders
    struct GlobalUBO {
        glm::mat4 projectionView{1.f}; // 4*4*4 = 4*16, alignment ok
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.1f};
        glm::vec4 pointLightPosition{-1.f, -1.f, -1.f, 0.f};
        glm::vec4 pointLightColor{0.8f, 0.8f, 0.8f, 10.0f};
        glm::float32 time{0.f};
    };

    // Public
    App::App()
    {
        m_GlobalDescriptorPool = DescriptorPool::Builder(mr_Device)
            .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        m_LoadCubes();
    }

    App::~App() {};

    void App::run() {

        // Data on the GPU
        Buffer globalUbo{
            mr_Device,
            sizeof(GlobalUBO),
            SwapChain::MAX_FRAMES_IN_FLIGHT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            mr_Device.properties.limits.minUniformBufferOffsetAlignment};
        globalUbo.map();

        // Need to set up the this so that the DescriptorPool knows what to give out.
        std::unique_ptr<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(mr_Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();

        // A descriptor set for even and odd frames.
        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = globalUbo.descriptorInfoForIndex(i);
            DescriptorWriter(*globalSetLayout, *m_GlobalDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{mr_Device, m_Renderer.p_GetSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        if (glfwRawMouseMotionSupported()) {
          glfwSetInputMode(m_Window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          glfwSetInputMode(m_Window.getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        auto viewerObject = GameObject::CreateGameObject();
        KeyboardMovementController cameraController{};
        std::cout << __cplusplus << "\n";
        std::cout << "Push constants limit " << mr_Device.properties.limits.maxPushConstantsSize << "\n";

        auto previousTime = std::chrono::high_resolution_clock::now();
        auto mousePrevious = m_Window.getMousePosition();
        float elapsedTime = 0.f;

        while (!m_Window.shouldClose()) {

            // This checks for clicks and stuff.
            glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - previousTime).count();
            previousTime = currentTime;
            elapsedTime += glm::mod(frameTime, glm::two_pi<float>());

            auto mouseCurrent = m_Window.getMousePosition();
            auto mouseDelta = mouseCurrent - mousePrevious;
            mousePrevious = mouseCurrent;

            cameraController.moveInPlaneXZ(m_Window.getWindow(), frameTime, viewerObject);
            cameraController.rotateWithMouse(m_Window.getWindow(), frameTime, viewerObject, mouseDelta);
            camera.setViewYXZ(viewerObject.p_Transform.translation, viewerObject.p_Transform.rotation);
            float aspectRatio = m_Renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 100.f);

            if(auto commandBuffer = m_Renderer.beginFrame()) {
                int backFrame = m_Renderer.getCurrentFrameIndex();
                FrameInfo frameInfo{backFrame, frameTime, commandBuffer, camera, globalDescriptorSets[backFrame]};

                // Update
                GlobalUBO stagingUBO{};
                stagingUBO.projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();
                stagingUBO.time = elapsedTime;
                globalUbo.writeToIndex(&stagingUBO, frameInfo.backFrame);
                globalUbo.flushIndex(backFrame);

                // Render
                m_Renderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.m_RenderGameObjects(frameInfo, m_GameObjects);
                m_Renderer.endSwapChainRenderPass(commandBuffer);
                m_Renderer.endFrame();
            };
        }

        // Block until GPU finishes execution.
        vkDeviceWaitIdle(mr_Device.device());
    };


    void App::m_LoadCubes() {
        std::shared_ptr<Model> whiteModel = Model::CreateModelFromFile(mr_Device, "models/white_cube.obj");
        std::shared_ptr<Model> blueModel = Model::CreateModelFromFile(mr_Device, "models/blue_cube.obj");
        std::shared_ptr<Model> quadModel = Model::CreateModelFromFile(mr_Device, "models/quad.obj");

        std::vector<GameObject> whiteCubes;
        std::vector<GameObject> blueCubes;
        const std::size_t count = 42;
        for (int i = 0; i < count; i++) {
            auto cube = GameObject::CreateGameObject();
            cube.model = whiteModel;
            cube.p_Transform.scale = {.05f, .05f, .05f};
            whiteCubes.push_back(std::move(cube));

            cube = GameObject::CreateGameObject();
            cube.model = blueModel;
            cube.p_Transform.scale = {.05f, .05f, .05f};
            blueCubes.push_back(std::move(cube));
        }

        // A
        glm::vec3 offset = {-1.75f, 0.f, 0.f};
        std::size_t i = 0;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.15f, .0f, 0.00f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.15f, .0f, 0.15f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.15f, .0f, 0.30f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.40f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{-.15f, .0f, 0.00f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{-.15f, .0f, 0.15f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{-.15f, .0f, 0.30f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.15f} + offset;

        // I
        offset = {-1.25f, 0.f, 0.f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.20f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.40f} + offset;

        std::size_t n = 0;
        // L
        offset = {-.75f, 0.f, 0.f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.40f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.20f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.2f, .0f, 0.00f} + offset;

        // i
        offset = {-0.3f, 0.f, 0.f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.15f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;

        // v
        offset = {-0.0f, 0.f, 0.f};
        blueCubes[n++].p_Transform.translation = glm::vec3{-.1f, .0f, 0.150f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.1f, .0f, 0.15f} + offset;
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;

        // e
        offset = {.2f, 0.f, 0.f};
        blueCubes[n].p_Transform.scale = {.03f, .03f, .03f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.20f} + offset;
        blueCubes[n].p_Transform.scale = {.03f, .03f, .03f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.10f} + offset;
        blueCubes[n].p_Transform.scale = {.03f, .03f, .03f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;
        blueCubes[n].p_Transform.scale = {.025f, .025f, .025};
        blueCubes[n++].p_Transform.translation = glm::vec3{.075f, .0f, 0.10f} + offset;
        blueCubes[n].p_Transform.scale = {.03f, .03f, .03f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.1f, .0f, 0.20f} + offset;
        blueCubes[n].p_Transform.scale = {.03f, .03f, .03f};
        blueCubes[n++].p_Transform.translation = glm::vec3{.1f, .0f, 0.00f} + offset;

        // S
        offset = {0.7f, 0.f, 0.f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.15f, .0f, 0.4f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{0.f, .0f, 0.4f} + offset;
        whiteCubes[i].p_Transform.scale = {.03f, .03f, .03f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.025f, .0f, 0.3f} + offset;
        whiteCubes[i].p_Transform.scale = {.03f, .03f, .03f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.05f, .0f, 0.25f} + offset;
        whiteCubes[i].p_Transform.scale = {.03f, .03f, .03f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.10f, .0f, 0.20f} + offset;
        whiteCubes[i].p_Transform.scale = {.03f, .03f, .03f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.125f, .0f, 0.15f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.15f, .0f, 0.0f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.0f} + offset;

        // i
        offset = {1.2f, 0.f, 0.f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.15f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.00f} + offset;

        // m
        offset = {1.5f, 0.f, 0.f};
        whiteCubes[i++].p_Transform.translation = glm::vec3{.0f, .0f, 0.0f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.1f, .0f, 0.1f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.2f, .0f, 0.05f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.3f, .0f, 0.1f} + offset;
        whiteCubes[i++].p_Transform.translation = glm::vec3{.4f, .0f, 0.0f} + offset;

        for (int j = 0; j < i; j++) {
            m_GameObjects.push_back(std::move(whiteCubes[j]));
        }
        for (int j = 0; j < n; j++) {
            m_GameObjects.push_back(std::move(blueCubes[j]));
        }

        auto quad = GameObject::CreateGameObject();
        quad.model = quadModel;
        quad.p_Transform.translation = {0.f, 0.2f, 0.f};
        quad.p_Transform.scale = {10.f, 1.f, 10.f};
        m_GameObjects.push_back(std::move(quad));
    }


    // Private
    void App::m_LoadGameObjects() {
        std::shared_ptr<Model> model = Model::CreateModelFromFile(mr_Device, "models/colored_cube.obj");
        std::shared_ptr<Model> another_model = Model::CreateModelFromFile(mr_Device, "models/smooth_vase.obj");
        std::shared_ptr<Model> flat_model = Model::CreateModelFromFile(mr_Device, "models/flat_vase.obj");
        std::shared_ptr<Model> torso_model = Model::CreateModelFromFile(mr_Device, "models/the-valentini-torso_bronze.obj");
        std::shared_ptr<Model> quad_model = Model::CreateModelFromFile(mr_Device, "models/quad.obj");

        auto cube = GameObject::CreateGameObject();
        auto another_cube = GameObject::CreateGameObject();
        auto flat_vase = GameObject::CreateGameObject();
        auto torso = GameObject::CreateGameObject();
        auto quad = GameObject::CreateGameObject();

        cube.model = model;
        another_cube.model = another_model;
        flat_vase.model = flat_model;
        torso.model = torso_model;
        quad.model = quad_model;

        cube.p_Transform.translation = {.25f, .0f, 2.5f};
        cube.p_Transform.scale = {.2f, .2f, .2f};
        another_cube.p_Transform.translation = {-.25f, .2f, 4.f};
        another_cube.p_Transform.scale = {1.f, 1.f, 1.f};
        flat_vase.p_Transform.translation = {-1.f, .2f, 2.5f};
        flat_vase.p_Transform.scale = {1.f, 1.f, 1.f};
        torso.p_Transform.translation = {0.f, 0.2f, 5.f};
        torso.p_Transform.scale = 0.5f * glm::vec3{1.f, 1.f, 1.f};
        torso.p_Transform.rotation = glm::vec3{0.f, glm::pi<float>(), glm::pi<float>()};
        quad.p_Transform.translation = {0.f, 0.2f, 0.f};
        quad.p_Transform.scale = {10.f, 1.f, 10.f};

        m_GameObjects.push_back(std::move(cube));
        m_GameObjects.push_back(std::move(another_cube));
        m_GameObjects.push_back(std::move(flat_vase));
        m_GameObjects.push_back(std::move(torso));
        m_GameObjects.push_back(std::move(quad));
    };



    void App::createTextureImage() {
            int texWidth, texHeight, texChannels;
            stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            // VkBuffer stagingBuffer;
            // VkDeviceMemory stagingBufferMemory;

            Buffer stagingBuffer{
                mr_Device,
                4,
                static_cast<uint32_t>(texWidth) * static_cast<uint32_t>(texHeight),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

            stagingBuffer.map();
            stagingBuffer.writeToBuffer((void*)pixels);
            stbi_image_free(pixels);

            VkImage textureImage;
            VkDeviceMemory textureImageMemory;

            createImage(
                texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage,
                textureImageMemory);
    }

    void App::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(mr_Device.device(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(mr_Device.device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = mr_Device.findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(mr_Device.device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(mr_Device.device(), image, imageMemory, 0);
}

} // namespace teng
