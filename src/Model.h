#ifndef MELLIANCLIENT_MODEL_H
#define MELLIANCLIENT_MODEL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <cassert>
#include <glm/glm.hpp>
#include "Device.h"

class Model
{
public:
    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions()
        {
            return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            return {
                {0, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, position)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
            };
        }
    };

    Model(Device &device, const std::vector<Vertex> &vertices) : device{device}
    {
        createVertexBuffers(vertices);
    }

    ~Model()
    {
        vkDestroyBuffer(device.device(), vertex_buffer, nullptr);
        vkFreeMemory(device.device(), vertex_buffer_memory, nullptr);
    }

    Model(const Model &) = delete;

    Model &operator=(Model &&) = delete;

    void bind(VkCommandBuffer command_buffer)
    {
        VkBuffer buffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
    }

    void draw(VkCommandBuffer command_buffer)
    {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }

private:
    void createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertex_count = static_cast<uint32_t>(vertices.size());

        assert(vertex_count >= 3 && "vertex must be at least 3");

        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;

        device.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertex_buffer,
            vertex_buffer_memory
        );

        void *data;

        vkMapMemory(device.device(), vertex_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(device.device(), vertex_buffer_memory);
    }

    Device &device;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    uint32_t vertex_count;
};

#endif //MELLIANCLIENT_MODEL_H
