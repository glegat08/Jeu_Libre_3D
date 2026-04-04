#pragma once
#include "Global.h"

/**
 * @brief Standard mesh vertex used for 3D rendering.
 *
 * Contains position, normal, color, UV coordinates, tangent for normal mapping, and joint data for skeletal animation.
 * Also provides Vulkan binding and attribute descriptions
 * for pipeline vertex input configuration.
 */
struct Vertex
{
    glm::vec3 pos;      ///< Vertex position in object space.
    glm::vec3 normal;   ///< Vertex normal vector.
    glm::vec4 color;    ///< Vertex color (RGBA).
    glm::vec2 uv;       ///< Texture coordinates.
	glm::vec4 tangent;  ///< Vertex tangent vector (for normal mapping).
	glm::ivec4 joints;  ///< Joint indices for skeletal animation (up to 4 joints).
	glm::vec4 weights;  ///< Joint weights for skeletal animation (corresponding to joints).

    /**
     * @brief Returns the Vulkan binding description for this vertex type.
     *
     * Binding 0, stride = sizeof(Vertex), per‑vertex input rate.
     */
    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
    }

    /**
     * @brief Returns the Vulkan attribute descriptions for each vertex attribute.
     *
     * Layout:
     * - location 0 → position (vec3)
     * - location 1 → normal   (vec3)
     * - location 2 → color    (vec4)
     * - location 3 → uv       (vec2)
	 * - location 4 → tangent  (vec4)
	 * - location 5 → joints   (ivec4)
	 * - location 6 → weights  (vec4)
     */
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        return 
        {
            vk::VertexInputAttributeDescription(
                0, 0, vk::Format::eR32G32B32Sfloat,     offsetof(Vertex, pos)),
            vk::VertexInputAttributeDescription(
                1, 0, vk::Format::eR32G32B32Sfloat,     offsetof(Vertex, normal)),
            vk::VertexInputAttributeDescription(
                2, 0, vk::Format::eR32G32B32A32Sfloat,  offsetof(Vertex, color)),
            vk::VertexInputAttributeDescription(
                3, 0, vk::Format::eR32G32Sfloat,        offsetof(Vertex, uv)),
            vk::VertexInputAttributeDescription(
                4, 0, vk::Format::eR32G32B32A32Sfloat,  offsetof(Vertex, tangent)),
            vk::VertexInputAttributeDescription(
                5, 0, vk::Format::eR32G32B32A32Sint,    offsetof(Vertex, joints)),
            vk::VertexInputAttributeDescription(
                6, 0, vk::Format::eR32G32B32A32Sfloat,  offsetof(Vertex, weights))
        };
    }

    
    /**
     * @brief Equality operator for hashing and deduplication.
     *
     * @note Normal is intentionally not compared.
     */
    bool operator==(const Vertex& other) const
    {
        return pos == other.pos &&
            color == other.color &&
            uv == other.uv && normal == other.normal;
    }
};

/**
 * @brief Hash specialization for Vertex to allow use in unordered containers.
 */
template <>
struct std::hash<Vertex>
{
    size_t operator()(Vertex const& vertex) const noexcept
    {
        return ((hash<glm::vec3>()(vertex.pos)
            ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
            ^ (hash<glm::vec2>()(vertex.uv) << 1);
    }
};


struct Vertex2D
{
    glm::vec2 pos;
    glm::vec2 uv;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return { 0, sizeof(Vertex2D), vk::VertexInputRate::eVertex };
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        return {
            vk::VertexInputAttributeDescription(
                0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex2D, pos)),
            vk::VertexInputAttributeDescription(
                1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex2D, uv))
        };
    }

};

struct UiData
{
    struct UiValidData
    {
        glm::vec4 color = { 1,1,1,1 };
        glm::vec4 raw1;
        glm::vec4 raw2;
        glm::vec4 raw3;


    };
    glm::vec4 color = { 1,1,1,1 };
    glm::mat3 model = glm::identity<glm::mat3>();
    UiValidData GetValid() const 
    {
        return UiValidData{
            .color = color,
            .raw1 = glm::vec4(model[0][0], model[1][0], model[2][0], 1.0f),
        	.raw2 = glm::vec4(model[0][1], model[1][1], model[2][1], 1.0f),
			.raw3 = glm::vec4(model[0][2], model[1][2], model[2][2], 1.0f)
        };
    }
};

/**
 * @brief Simple vertex used for line rendering (debug segments, gizmos, etc.).
 *
 * Contains only position and color.
 */
struct SegmentVertex
{
    glm::vec3 pos;   ///< Vertex position.
    glm::vec4 color; ///< Vertex color (RGBA).

    /**
     * @brief Returns the Vulkan binding description for this vertex type.
     */
    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return { 0, sizeof(SegmentVertex), vk::VertexInputRate::eVertex };
    }

    /**
     * @brief Returns the Vulkan attribute descriptions for this vertex type.
     *
     * Layout:
     * - location 0 → position (vec3)
     * - location 1 → color    (vec4)
     */
    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        return {
            vk::VertexInputAttributeDescription(
                0, 0, vk::Format::eR32G32B32Sfloat, offsetof(SegmentVertex, pos)),
            vk::VertexInputAttributeDescription(
                1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(SegmentVertex, color))
        };
    }

    /// @brief Equality operator for hashing and deduplication.
    bool operator==(const SegmentVertex& other) const
    {
        return pos == other.pos && color == other.color;
    }
};

/**
 * @brief Hash specialization for SegmentVertex.
 */
template <>
struct std::hash<SegmentVertex>
{
    size_t operator()(SegmentVertex const& vertex) const noexcept
    {
        return ((hash<glm::vec3>()(vertex.pos)
            ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1);
    }
};