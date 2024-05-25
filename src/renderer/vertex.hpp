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


struct VertexPCNT {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            0,                              // binding
            sizeof(VertexPCNT),             // stride
            vk::VertexInputRate::eVertex    // inputRate
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0] = vk::VertexInputAttributeDescription(
            0,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPCNT, pos)       // offset
        );

        attributeDescriptions[1] = vk::VertexInputAttributeDescription(
            1,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPCNT, color)     // offset
        );

        attributeDescriptions[2] = vk::VertexInputAttributeDescription(
            2,                              // location
            0,                              // binding
            vk::Format::eR32G32Sfloat,      // format
            offsetof(VertexPCNT, texCoord)  // offset
        );

        attributeDescriptions[3] = vk::VertexInputAttributeDescription(
            3,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPCNT, normal)    // offset
        );

        return attributeDescriptions;
    }

    bool operator==(const VertexPCNT& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
    }
};


struct VertexPTN {
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return vk::VertexInputBindingDescription(
            0,                              // binding
            sizeof(VertexPTN),              // stride
            vk::VertexInputRate::eVertex    // inputRate
        );
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0] = vk::VertexInputAttributeDescription(
            0,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPTN, pos)        // offset
        );

        attributeDescriptions[1] = vk::VertexInputAttributeDescription(
            1,                              // location
            0,                              // binding
            vk::Format::eR32G32Sfloat,      // format
            offsetof(VertexPTN, texCoord)   // offset
        );

        attributeDescriptions[2] = vk::VertexInputAttributeDescription(
            2,                              // location
            0,                              // binding
            vk::Format::eR32G32B32Sfloat,   // format
            offsetof(VertexPTN, normal)     // offset
        );

        return attributeDescriptions;
    }

    bool operator==(const VertexPTN& other) const {
        return pos == other.pos && texCoord == other.texCoord && normal == other.normal;
    }
};

}