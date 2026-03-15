#pragma once
#include <fstream>
#include <glm/glm.hpp>
#include "Core/RessourcesManager.h"

/**
 * @brief Loads a binary file into an std::ifstream wrapped in a unique_ptr.
 *
 * Opens the file in:
 * - binary mode
 * - at end (ios::ate) to allow size queries
 *
 * @param filePath Absolute or relative path to the file.
 * @return A unique_ptr to an opened std::ifstream.
 *
 * @throws std::runtime_error If the file cannot be opened.
 */
inline std::unique_ptr<std::ifstream> LoadFile(const std::string& filePath)
{
    auto file = std::make_unique<std::ifstream>(filePath, std::ios::ate | std::ios::binary);
    if (!file->is_open())
        throw std::runtime_error("failed to open file!");

    return std::move(file);
}

/**
 * @brief Resource manager for binary file streams.
 *
 * Uses:
 * - std::ifstream as the resource type
 * - no extra construction arguments
 * - LoadFile as the loading function
 */
using FileManager = KGR::ResourceManager<std::ifstream, KGR::TypeWrapper<>, LoadFile>;

#include "stb_image.h"

/**
 * @brief Simple wrapper around stb_image for loading image files.
 *
 * Stores:
 * - width, height, channels
 * - raw pixel data (stbi_uc*)
 * - file path
 *
 * Provides move semantics but disables copying.
 */
struct Image
{
    /**
     * @brief Loads an image from disk using stb_image.
     *
     * @param filePath Path to the image file.
     */
    Image(const std::string& filePath);

    /**
     * @brief Frees pixel memory using stbi_image_free().
     */
    ~Image();

    int width = 0;       ///< Image width in pixels.
    int height = 0;      ///< Image height in pixels.
    int channels = 0;    ///< Number of color channels.
    stbi_uc* pixels = nullptr; ///< Raw pixel data.
    std::string path;    ///< Original file path.

    Image(const Image&) = delete;
    Image(Image&& other) noexcept;

    Image& operator=(const Image&) = delete;
    Image& operator=(Image&& other) noexcept;
};

/**
 * @brief Loads an Image using stb_image and returns a unique_ptr.
 */
std::unique_ptr<Image> LoadFile2(const std::string& filePath);

/**
 * @brief Resource manager for STB-loaded images.
 */
using STBManager = KGR::ResourceManager<Image, KGR::TypeWrapper<>, LoadFile2>;

#define GLM_ENABLE_EXPERIMENTAL
#include <tiny_obj_loader.h>
#include <glm/gtx/hash.hpp>

/**
 * @brief Vertex structure used for OBJ loading.
 *
 * Contains:
 * - position
 * - color
 * - texture coordinates
 *
 * Provides equality and hashing for deduplication.
 */
struct Vertex2
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const Vertex2& other) const
    {
        return pos == other.pos &&
            color == other.color &&
            texCoord == other.texCoord;
    }
};

/**
 * @brief Hash specialization for Vertex2 to allow use in unordered containers.
 */
template <>
struct std::hash<Vertex2>
{
    size_t operator()(Vertex2 const& vertex) const noexcept
    {
        return ((hash<glm::vec3>()(vertex.pos)
            ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1)
            ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

/**
 * @brief Simple OBJ container storing vertices and indices.
 *
 * Used as an intermediate structure before converting to engine Mesh/SubMeshes.
 */
struct Obj
{
    std::vector<Vertex2> vertices;
    std::vector<uint32_t> indices;
};

/**
 * @brief Loads an OBJ file using tinyobjloader and returns a parsed Obj structure.
 *
 * @param filePath Path to the OBJ file.
 * @return A unique_ptr to an Obj containing vertices and indices.
 */
std::unique_ptr<Obj> LoadFile3(const std::string& filePath);