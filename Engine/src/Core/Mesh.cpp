#include"Core/Mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <nlohmann/json.hpp>
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>



#include "Hasher.h"
#include "VulkanCore.h"


SubMeshes::~SubMeshes()
{

}

SubMeshes::SubMeshes(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const std::string tag, KGR::_Vulkan::VulkanCore* core) : m_vertices(vertices),m_indices(indices),m_id(Hash::FNV1aHash(tag.c_str(),tag.size()))
{
	m_vertexBuffer = core->CreateVertexBuffer(vertices);
	m_indexBuffer = core->CreateIndexBuffer(indices);
}

std::vector<Vertex> SubMeshes::GetVertices() const
{
	return m_vertices;
}

std::vector<uint32_t> SubMeshes::GetIndex() const
{
	return m_indices;
}

size_t SubMeshes::VertexCount() const
{
	return m_vertices.size();
}

size_t SubMeshes::IndexCount() const
{
	return m_indices.size();
}

uint64_t SubMeshes::GetID() const
{
	return m_id;
}


void SubMeshes::BindVertices(const vk::raii::CommandBuffer* buffer)
{
	buffer->bindVertexBuffers(0, *m_vertexBuffer.Get(), { 0 });
}

void SubMeshes::BindIndices(const vk::raii::CommandBuffer* buffer)
{
	buffer->bindIndexBuffer(*m_indexBuffer.Get(), 0, vk::IndexType::eUint32);
}



uint32_t Mesh::GetSubMeshesCount() const
{
	return m_subMeshes.size();
}

const SubMeshes& Mesh::GetSubMesh(const std::string& name) const 
{
	uint64_t hash = Hash::FNV1aHash(name.c_str(), name.size());
	auto it = std::find_if(m_subMeshes.begin(), m_subMeshes.end(), [hash](const std::unique_ptr<SubMeshes>& sub)
		{
			return sub->GetID() == hash;
		});
	if (it != m_subMeshes.end())
		return *it->get();
	throw std::out_of_range("no mesh found");
}

const SubMeshes& Mesh::GetSubMesh(uint32_t id) const
{
	return *m_subMeshes[id];
}

void Mesh::Bind(const vk::raii::CommandBuffer* buffer, uint32_t index)
{
	m_subMeshes[index]->BindVertices(buffer);
	m_subMeshes[index]->BindIndices(buffer);
}

void Mesh::AddSubMesh(std::unique_ptr<SubMeshes> mesh)
{
	m_subMeshes.push_back(std::move(mesh));
}

std::unique_ptr<Mesh> LoadMesh(const std::string& filePath, KGR::_Vulkan::VulkanCore* core)
{
	std::unique_ptr<Mesh> result = std::make_unique<Mesh>();

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn, err;

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(),nullptr,true))
	{
		throw std::runtime_error(warn + err);
	}
	for (const auto& shape : shapes)
	{
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		std::vector<Vertex> allVertices;
		std::vector<uint32_t> indices;
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;

			//POS
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2] };

			// Normal if available
			if (index.normal_index >= 0)
			{
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			// UV if available 
			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}
			vertex.color = glm::vec4{ 1,1,1,1};

			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(allVertices.size());
				allVertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
		result->AddSubMesh(std::make_unique<SubMeshes>(allVertices, indices,shape.name, core));
	}
	return std::move(result);

}

