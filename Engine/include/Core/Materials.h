#pragma once
#include "Texture.h"

struct Material
{
	Texture* baseColor = nullptr;
	Texture* emissive = nullptr;
	Texture* normalMap = nullptr;
	Texture* pbrMap = nullptr;
};


struct MaterialComponent
{
    MaterialComponent() = default;

    /**
     * @brief Resizes the internal texture array.
     *
     * @param size Number of textures (usually number of submeshes).
     */
    void SetSize(uint32_t size)
    {
        m_materials.resize(size);
    }

    /// @brief Returns the number of textures stored.
    size_t Size() const
    {
        return m_materials.size();
    }

    /**
     * @brief Assigns a texture to a specific index.
     *
     * @param index Submesh index.
     * @param texture Pointer to a Texture resource.
     */
    void AddMaterial(uint32_t index, Material texture)
    {
        m_materials[index] = texture;
    }

   
  

    Material GetMaterial(uint32_t index) const 
    {
        return m_materials[index];
    }

   
    std::vector<Material> GetAllMaterials() const
    {
        return m_materials;
    }

private:
    std::vector<Material> m_materials; 
};