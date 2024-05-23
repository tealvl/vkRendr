# pragma once

#include <glm/glm.hpp>
#include <array>
#include <vulkan/vulkan_raii.hpp>

namespace rendr{

struct VertexPCT {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            0,                              // binding
            sizeof(VertexPCT),                 // stride
            vk::VertexInputRate::eVertex    // inputRate
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0] = vk::VertexInputAttributeDescription(
            0,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPCT, pos)           // offset
        );

        attributeDescriptions[1] = vk::VertexInputAttributeDescription(
            1,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPCT, color)         // offset
        );

        attributeDescriptions[2] = vk::VertexInputAttributeDescription(
            2,                              // location
            0,                              // binding
            vk::Format::eR32G32Sfloat,      // format
            offsetof(VertexPCT, texCoord)      // offset
        );

        return attributeDescriptions;
    }

    bool operator==(const VertexPCT& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

}