#pragma once

#include "teng_device.hpp"
#include "teng_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace teng {

    class Model {

        public:
            struct Vertex {
                glm::vec3 position{};
                glm::vec3 color{};
                glm::vec3 normal{};
                glm::vec2 uv{};

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

                bool operator==(const Vertex& other) const {
                    return position == other.position and
                        color == other.color and
                        normal == other.normal and
                        uv == other.uv;
                };
            };

            // Temporary container for vertex and index data until they can be copied over to the model's vertex and index buffers.
            struct Data {
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};

                void loadModel(const std::string& objFile);

            };

            Model(Device &m_TengDevice, const Data& data);
            ~Model();

            Model(const Model&) = delete;
            Model &operator=(const Model&) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

            static std::unique_ptr<Model> CreateModelFromFile(Device& r_Device, const std::string& objFile);

            static Vertex midpoint(Vertex a, Vertex b);

        private:
            Device& m_Device;

            void m_CreateVertexBuffers(const std::vector<Vertex> &vertices);
            std::unique_ptr<Buffer> ma_VertexBuffer;
            uint32_t m_VertexCount;

            void m_CreateIndexBuffers(const std::vector<uint32_t> &indices);
            bool hasIndexBuffer{false};
            std::unique_ptr<Buffer> ma_IndexBuffer;
            uint32_t m_IndexCount;
    };

}
