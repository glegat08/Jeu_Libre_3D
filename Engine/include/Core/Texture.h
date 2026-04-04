#pragma once
#include "Global.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "DescriptorSet.h"
#include "Image.h"
#include "RessourcesManager.h"

namespace KGR::_Vulkan
{
    class VulkanCore;
}

/**
 * @brief GPU texture holding a Vulkan image and its descriptor set.
 */
class Texture
{
public:
    /**
     * @brief constructs a texture from a Vulkan image and its descriptor set.
     * @param image vulkan image resource.
     * @param set descriptor set referencing the image.
     */
    Texture(KGR::_Vulkan::Image&& image, KGR::_Vulkan::DescriptorSet&& set);

    /**
     * @brief binds the texture descriptor set to the given command buffer.
     * @param buffer command buffer used for rendering.
     * @param layout pipeline layout that owns the descriptor set.
     * @param set descriptor set index in the pipeline layout.
     */
    void Bind(const vk::raii::CommandBuffer* buffer,
        const vk::raii::PipelineLayout* layout,
        int set);

    glm::vec2 GetSize() const;

private:
    KGR::_Vulkan::Image         m_image;
    KGR::_Vulkan::DescriptorSet m_set;
};


struct TextureComponent
{
    Texture* texture;
};


/**
 * @brief loads a texture from an image file on disk.
 * @param filePath path to the image file.
 * @param core vulkan context used for GPU resource creation.
 * @return unique_ptr to the uploaded texture.
 */
std::unique_ptr<Texture> LoadTexture(const std::string& filePath,
    KGR::_Vulkan::VulkanCore* core);

/**
 * @brief creates a GPU texture from encoded image bytes (PNG, JPEG, etc.).
 * @param data pointer to the encoded image bytes.
 * @param size byte count.
 * @param core vulkan context.
 * @return unique_ptr to the uploaded texture.
 */
std::unique_ptr<Texture> LoadTextureFromMemory(const uint8_t* data,
    size_t size, KGR::_Vulkan::VulkanCore* core);

/**
 * @brief creates a GPU texture from a raw RGBA pixel buffer.
 * @param rgbaPixels pointer to the raw pixel data, 4 bytes per pixel.
 * @param width image width in pixels.
 * @param height image height in pixels.
 * @param core vulkan context.
 * @return unique_ptr to the uploaded texture.
 */
std::unique_ptr<Texture> LoadTextureRaw(const uint8_t* rgbaPixels,
    uint32_t width, uint32_t height, KGR::_Vulkan::VulkanCore* core);

/**
 * @brief resource manager for loading and caching file-based textures.
 */
using TextureLoader =
KGR::ResourceManager<Texture,
    KGR::TypeWrapper<KGR::_Vulkan::VulkanCore*>,
    LoadTexture>;