#pragma once
#include "Texture.h"

struct Material
{
	Texture* baseColor = nullptr;
	Texture* emissive = nullptr;
	Texture* normalMap = nullptr;
	Texture* pbrMap = nullptr;
};


/**
 * @brief Component that stores a collection of materials per submesh.
 *
 * This component manages a dynamic array of Material objects, typically
 * corresponding to the different submeshes of a mesh. Each submesh can
 * have its own material assigned.
 */
    struct MaterialComponent
{
    MaterialComponent() = default;

    /**
     * @brief Resizes the internal material array.
     *
     * This function adjusts the number of materials stored in the component.
     * It is usually called to match the number of submeshes in a mesh.
     *
     * @param size Number of materials (typically equal to submesh count).
     */
    void SetSize(uint32_t size)
    {
        m_materials.resize(size);
    }

    /**
     * @brief Gets the number of stored materials.
     *
     * @return The number of materials currently stored.
     */
    size_t Size() const
    {
        return m_materials.size();
    }

    /**
     * @brief Assigns a material to a specific index.
     *
     * Associates a Material with a given submesh index.
     *
     * @param index Index of the submesh.
     * @param texture Material to assign.
     */
    void AddMaterial(uint32_t index, Material texture)
    {
        m_materials[index] = texture;
    }

    /**
     * @brief Retrieves a material at a given index.
     *
     * @param index Index of the submesh.
     * @return The material associated with the given index.
     */
    Material GetMaterial(uint32_t index) const
    {
        return m_materials[index];
    }

    /**
     * @brief Retrieves all stored materials.
     *
     * @return A copy of the internal material array.
     */
    std::vector<Material> GetAllMaterials() const
    {
        return m_materials;
    }

private:
    /**
     * @brief Internal storage of materials.
     *
     * Each entry corresponds to a submesh material.
     */
    std::vector<Material> m_materials;
};