#pragma once

#include "teng_pipeline.hpp"
#include "teng_descriptors.hpp"
#include "teng_window.hpp"
#include "teng_model.hpp"
#include "teng_game_object.hpp"
#include "teng_renderer.hpp"

// std
#include <memory>
#include <vector>

namespace teng {

    const glm::vec3 RED{1.0f, 0.f, 0.f};
    const glm::vec3 GREEN{.0f, 1.f, 0.f};
    const glm::vec3 BLUE{0.0f, 0.f, 1.f};
    const glm::vec3 WHITE{1.0f, 1.f, 1.f};

    class App {

        public:

            static constexpr int WIDTH = 900;
            static constexpr int HEIGHT = 900;

            App();
            ~App();

            App(const App&) = delete;
            App &operator=(const App&) = delete;

            void run();
            void m_RunGravSystem();

        private:

            void m_LoadGameObjects();
            void m_LoadCubes();
            void m_LoadGravGameObjects();
            void m_Trapienski(
                Model::Vertex top,
                Model::Vertex rc,
                Model::Vertex  lc,
                u_int32_t depth,
                std::vector<Model::Vertex> *out);
            void m_Circle(uint32_t count, glm::vec2 translation = glm::vec2{0.f, 0.f},  glm::vec3 color = RED);
            std::unique_ptr<Model> m_Circle2(uint32_t count);
            std::unique_ptr<Model> createCubeModel(Device& r_Device, glm::vec3 offset);
            void createTextureImage();
            void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

            Window m_Window{WIDTH, HEIGHT, "Vulkan"}; // Creates a window when App is contructed.
            Device mr_Device{m_Window}; // Creates a device after window.
            Renderer m_Renderer{m_Window, mr_Device}; // Creates renderer after device.
            std::unique_ptr<DescriptorPool> m_GlobalDescriptorPool{};
            std::vector<GameObject> m_GameObjects;
};
} // namespace teng
