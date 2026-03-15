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
 * @brief GPU texture resource containing an image and its descriptor set.
 *
 * This class owns:
 * - a Vulkan image (GPU memory + sampler + view)
 * - a descriptor set used to bind the texture to shaders
 *
 * It provides a simple Bind() method for use during rendering.
 */
class Texture
{
public:
    /**
     * @brief Constructs a texture from an image and a descriptor set.
     *
     * @param image Vulkan image resource.
     * @param set Descriptor set referencing the image.
     */
    Texture(KGR::_Vulkan::Image&& image, KGR::_Vulkan::DescriptorSet&& set);

    /**
     * @brief Binds the texture to a command buffer.
     *
     * @param buffer Command buffer used for rendering.
     * @param layout Pipeline layout containing the descriptor set layout.
     * @param set Index of the descriptor set in the pipeline layout.
     */
    void Bind(const vk::raii::CommandBuffer* buffer,
        const vk::raii::PipelineLayout* layout,
        int set);

    glm::vec2 GetSize() const;
private:
    KGR::_Vulkan::Image m_image;        ///< GPU image resource.
    KGR::_Vulkan::DescriptorSet m_set;  ///< Descriptor set used for binding.
};


/**
 * @brief ECS component storing one or more textures for a mesh.
 *
 * A mesh may have multiple submeshes, each requiring its own texture.
 * This component stores a list of Texture* pointers indexed by submesh ID.
 */
struct TextureComponent
{
    TextureComponent() = default;

    /**
     * @brief Resizes the internal texture array.
     *
     * @param size Number of textures (usually number of submeshes).
     */
    void SetSize(uint32_t size)
    {
        m_textures.resize(size);
    }

    /// @brief Returns the number of textures stored.
    size_t Size() const
    {
        return m_textures.size();
    }

    /**
     * @brief Assigns a texture to a specific index.
     *
     * @param index Submesh index.
     * @param texture Pointer to a Texture resource.
     */
    void AddTexture(uint32_t index, Texture* texture)
    {
        m_textures[index] = texture;
    }

    /// @brief Returns a const pointer to the texture at the given index.
    const Texture* GetTexture(uint32_t index) const
    {
        return m_textures[index];
    }

    /// @brief Returns a pointer to the texture at the given index.
    Texture* GetTexture(uint32_t index)
    {
        return m_textures[index];
    }

    /// @brief Returns all textures as a modifiable vector.
    std::vector<Texture*>& GetAllTextures()
    {
        return m_textures;
    }

private:
    std::vector<Texture*> m_textures; ///< Texture list indexed by submesh.
};


/**
 * @brief Loads a texture from disk and creates a GPU texture resource.
 *
 * @param filePath Path to the texture file.
 * @param core VulkanCore instance used for GPU resource creation.
 * @return A unique_ptr to a newly created Texture.
 */
std::unique_ptr<Texture> LoadTexture(const std::string& filePath,
    KGR::_Vulkan::VulkanCore* core);

/**
 * @brief Resource manager alias for loading and caching textures.
 *
 * Uses:
 * - Texture as the resource type
 * - VulkanCore* as the dependency
 * - LoadTexture as the loading function
 */
using TextureLoader =
KGR::ResourceManager<Texture,
    KGR::TypeWrapper<KGR::_Vulkan::VulkanCore*>,
    LoadTexture>;

