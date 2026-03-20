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

std::vector<glm::vec3> s_ComputeTangents(
	const std::vector<glm::vec3>& positions,
	const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& texcoords,
	const std::vector<uint32_t>& indices
) {
	std::vector<glm::vec3> tangents(normals.size(), glm::vec3(0.0f));

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		// index 
		uint32_t i0 = indices[i];
		uint32_t i1 = indices[i + 1];
		uint32_t i2 = indices[i + 2];

		const glm::vec3& p0 = positions[i0];
		const glm::vec3& p1 = positions[i1];
		const glm::vec3& p2 = positions[i2];
		const glm::vec2& uv0 = texcoords[i0];
		const glm::vec2& uv1 = texcoords[i1];
		const glm::vec2& uv2 = texcoords[i2];

		glm::vec3 edge1 = p1 - p0;
		glm::vec3 edge2 = p2 - p0;
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		if (std::isfinite(f))
		{
			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			tangent = glm::normalize(tangent);

			float sx = deltaUV1.x;
			float sy = deltaUV2.x;
			float tx = deltaUV1.y;
			float ty = deltaUV2.y;

			
			tangents[i0] = tangent;
			tangents[i1] = tangent;
			tangents[i2] = tangent;
		}

		for (size_t i = 0; i < tangents.size(); ++i)
		{
			const glm::vec3& n = normals[i];
			glm::vec3& t = tangents[i];

			// Gram-Schmidt orthogonalization
			t = glm::normalize(t - n * glm::dot(n, t));

			if (glm::length2(t) < 0.001f)
			{
				if (glm::abs(n.x) < 0.9f)
					t = glm::normalize(glm::cross(n, glm::vec3(1.0f, 0.0f, 0.0f)));
				else
					t = glm::normalize(glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f)));
			}
		}

	}

	return tangents;
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
		std::vector<glm::vec3> posS;
		std::vector<glm::vec3> normalS;
		std::vector<glm::vec2> uvS;
		for (auto& v :allVertices)
		{
			posS.push_back(v.pos);
			normalS.push_back(v.normal);
			uvS.push_back(v.uv);
		}
		auto tangent = s_ComputeTangents(posS, normalS, uvS, indices);

		for (int i = 0 ; i < tangent.size(); ++i)
		{
			auto t = tangent[i];
			allVertices[i].tangent = { t.x,t.y,t.z,1 };
		}
		result->AddSubMesh(std::make_unique<SubMeshes>(allVertices, indices,shape.name, core));
	}
	return std::move(result);
}

