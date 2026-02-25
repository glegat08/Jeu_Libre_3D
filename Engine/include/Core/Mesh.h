#pragma once
#include "Global.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "Buffer.h"
#include "RessourcesManager.h"
#include "Vertex.h"


namespace KGR::_Vulkan
{
	class VulkanCore;
}

class SubMeshes
{
public:
	~SubMeshes();
	SubMeshes(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const std::string tag,KGR::_Vulkan::VulkanCore* core);
	std::vector<Vertex> GetVertices() const;
	std::vector<uint32_t> GetIndex() const ;
	size_t VertexCount() const;
	size_t IndexCount() const;
	uint64_t GetID() const;
	
	void BindVertices(const vk::raii::CommandBuffer* buffer);
	void BindIndices(const vk::raii::CommandBuffer* buffer);
private:
	std::vector<Vertex> m_vertices;
	KGR::_Vulkan::Buffer m_vertexBuffer;
	std::vector<uint32_t> m_indices;
	KGR::_Vulkan::Buffer m_indexBuffer;
	uint64_t m_id;
};

class Mesh
{
public:
	Mesh() = default;
	uint32_t GetSubMeshesCount() const;
	const SubMeshes& GetSubMesh(const std::string& name) const;
	const SubMeshes& GetSubMesh(uint32_t id) const;
	void Bind(const vk::raii::CommandBuffer* buffer, uint32_t index);
	void AddSubMesh(std::unique_ptr<SubMeshes> mesh);
private:
	std::vector <std::unique_ptr<SubMeshes>> m_subMeshes;
};

struct MeshComponent
{
	Mesh* mesh = nullptr;
};

std::unique_ptr<Mesh> LoadMesh(const std::string& filePat, KGR::_Vulkan::VulkanCore* core);


using MeshLoader = KGR::ResourceManager<Mesh, KGR::TypeWrapper<KGR::_Vulkan::VulkanCore*>, LoadMesh>;