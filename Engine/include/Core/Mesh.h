#pragma once
#include "Global.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "Buffer.h"
#include "RessourcesManager.h"
#include "Vertex.h"
#include "GLBLoader.h"

namespace KGR::_Vulkan
{
    class VulkanCore;
}

class Mesh;

/**
 * @brief Represents a single submesh containing vertices, indices, and GPU buffers.
 *
 * A mesh may be composed of multiple submeshes, each with its own:
 * - vertex buffer
 * - index buffer
 * - material/texture
 * - unique ID
 *
 * SubMeshes owns its GPU buffers and uploads data during construction.
 */
class SubMeshes
{
public:
    friend Mesh;

    /**
     * @brief Destroys GPU buffers associated with this submesh.
     */
    ~SubMeshes();

    /**
     * @brief Constructs a submesh from CPU vertex/index data.
     *
     * @param vertices CPU-side vertex array.
     * @param indices CPU-side index array.
     * @param tag Name or identifier for debugging.
     * @param core VulkanCore instance used to create GPU buffers.
     */
    SubMeshes(std::vector<Vertex> vertices,
        std::vector<uint32_t> indices,
        const std::string tag,
        KGR::_Vulkan::VulkanCore* core);

    /// @brief Returns a copy of the CPU-side vertex array.
    std::vector<Vertex> GetVertices() const;

    /// @brief Returns a copy of the CPU-side index array.
    std::vector<uint32_t> GetIndex() const;

    /// @brief Returns the number of vertices.
    size_t VertexCount() const;

    /// @brief Returns the number of indices.
    size_t IndexCount() const;

    /// @brief Returns the unique ID of this submesh.
    uint64_t GetID() const;

private:
    std::vector<Vertex> m_vertices;          ///< CPU-side vertex data.
    KGR::_Vulkan::Buffer m_vertexBuffer;     ///< GPU vertex buffer.

    std::vector<uint32_t> m_indices;         ///< CPU-side index data.
    KGR::_Vulkan::Buffer m_indexBuffer;      ///< GPU index buffer.

    uint64_t m_id;                           ///< Unique submesh identifier.

    /// @brief Binds the vertex buffer to the command buffer.
    void BindVertices(const vk::raii::CommandBuffer* buffer);

    /// @brief Binds the index buffer to the command buffer.
    void BindIndices(const vk::raii::CommandBuffer* buffer);
};


/**
 * @brief Represents a full mesh composed of multiple submeshes.
 *
 * Provides:
 * - access by index or name
 * - iteration over submeshes
 * - binding of submesh buffers during rendering
 */
class Mesh
{
public:
    using iterator = std::vector<std::unique_ptr<SubMeshes>>;

    friend KGR::_Vulkan::VulkanCore;

    Mesh() = default;

    /// @brief Returns the number of submeshes.
    uint32_t GetSubMeshesCount() const;

    /**
     * @brief Returns a submesh by name.
     *
     * @throws std::out_of_range If no submesh matches the name.
     */
    const SubMeshes& GetSubMesh(const std::string& name) const;

    /**
     * @brief Returns a submesh by index.
     *
     * @throws std::out_of_range If index is invalid.
     */
    const SubMeshes& GetSubMesh(uint32_t id) const;

    /**
     * @brief Adds a new submesh to the mesh.
     *
     * @param mesh Unique pointer to a SubMeshes instance.
     */
    void AddSubMesh(std::unique_ptr<SubMeshes> mesh);

private:
    std::vector<std::unique_ptr<SubMeshes>> m_subMeshes; ///< All submeshes.

    /**
     * @brief Binds the vertex and index buffers of a submesh.
     *
     * @param buffer Command buffer.
     * @param index Index of the submesh to bind.
     */
    void Bind(const vk::raii::CommandBuffer* buffer, uint32_t index);
};


/**
 * @brief ECS component storing a pointer to a mesh resource.
 */
struct MeshComponent
{
	Mesh* mesh = nullptr;
	std::string sourcePath;
};


/**
 * @brief Loads a mesh from disk and creates GPU buffers for all submeshes.
 *
 * @param filePath Path to the mesh file.
 * @param core VulkanCore instance used for buffer creation.
 * @return A unique_ptr to a fully loaded Mesh.
 */
std::unique_ptr<Mesh> LoadMesh(const std::string& filePath,
    KGR::_Vulkan::VulkanCore* core);

std::unique_ptr<Mesh> LoadMeshFromGLB(
    KGR::GLB::GLB_Loader& loader,
    KGR::_Vulkan::VulkanCore* vkCore);

/**
 * @brief Resource manager alias for loading and caching meshes.
 *
 * Uses:
 * - Mesh as the resource type
 * - VulkanCore* as the dependency
 * - LoadMesh as the loading function
 */
using MeshLoader =
KGR::ResourceManager<Mesh,
    KGR::TypeWrapper<KGR::_Vulkan::VulkanCore*>,
    LoadMesh>;