#include "teng_model.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include "utils.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
    template <>
    struct hash<teng::Model::Vertex> {
        size_t operator()(teng::Model::Vertex const& vertex) const {
            size_t seed = 0;
            teng::hashCombine(seed, vertex.color, vertex.normal, vertex.position, vertex.uv);
            return seed;
        }
    };
}

namespace teng {

    // VERTEX

    // Bindings tell the pipeline where to look for vertex attribute data.
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescription(1);
        bindingDescription[0].binding = 0;
        bindingDescription[0].stride = sizeof(Vertex);
        bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    };


    // Attributes define the format of vertex data.
    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescription{};

        uint32_t location = 0;
        attributeDescription.push_back({location++, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescription.push_back({location++, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescription.push_back({location++, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescription.push_back({location++, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescription;
    };

    // MODEL

    // Public
    Model::Model(Device &device, const Data& data) : m_Device{device} {
        m_CreateVertexBuffers(data.vertices);
        m_CreateIndexBuffers(data.indices);
    };

    Model::~Model() {};

    std::unique_ptr<Model> Model::CreateModelFromFile(Device& r_Device, const std::string& objFile) {
        Data data{};
        data.loadModel(objFile);
        std::cout << "Vertex count: " << data.vertices.size() << "\n";
        return std::make_unique<Model>(r_Device, data);
    };

    // Private
    // Copies vertex data from the to the GPU.
    void Model::m_CreateVertexBuffers(const std::vector<Vertex> &vertices) {

        // GPU buffer initialization.
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3.");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{
        m_Device,
        vertexSize,
        m_VertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        ma_VertexBuffer = std::make_unique<Buffer>(
        m_Device,
        vertexSize,
        m_VertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), ma_VertexBuffer->getBuffer(), bufferSize);
    };

    void Model::bind(VkCommandBuffer pCommandBuffer) {
        VkBuffer buffers[] = {ma_VertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, buffers, offsets);

        if(hasIndexBuffer) {
            vkCmdBindIndexBuffer(pCommandBuffer, ma_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        };
    };

    void Model::draw(VkCommandBuffer pCommandBuffer) {
        if(hasIndexBuffer) {
            vkCmdDrawIndexed(pCommandBuffer, m_IndexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(pCommandBuffer, m_VertexCount, 1, 0, 0);
        };
    };

    void Model::m_CreateIndexBuffers(const std::vector<uint32_t> &indices) {

        // GPU buffer initialization.
        m_IndexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = m_IndexCount > 0;

        if(!hasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
        m_Device,
        indexSize,
        m_IndexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        ma_IndexBuffer = std::make_unique<Buffer>(
        m_Device,
        indexSize,
        m_IndexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.copyBuffer(stagingBuffer.getBuffer(), ma_IndexBuffer->getBuffer(), bufferSize);
    };

    void Model::Data::loadModel(const std::string& objFile) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFile.c_str())) {
            throw std::runtime_error(warn + err);
        };

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for(const auto &shape : shapes) {
            for(const auto &index : shape.mesh.indices) {
                Vertex vertex{};

                if(index.vertex_index >= 0) {
                    vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                    vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };};

                if(index.normal_index >= 0) {
                    vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };}

                if(index.texcoord_index >= 0) {
                    vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };}

                if(uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}
