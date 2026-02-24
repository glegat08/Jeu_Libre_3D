#pragma once
#include "Global.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

struct Vertex3
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 color;
	glm::vec2 uv;

	static vk::VertexInputBindingDescription getBindingDescription() {
		return { 0, sizeof(Vertex3), vk::VertexInputRate::eVertex };
	}
	static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
		return {
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex3, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex3, normal)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex3, color)),
			vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex3, uv))
		};

	}
	bool operator==(const Vertex3& other) const
	{
		return pos == other.pos && color == other.color && uv == other.uv;
	}
};
class Mesh
{
	
};