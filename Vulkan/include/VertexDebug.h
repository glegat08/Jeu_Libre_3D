#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace KGR
{
	namespace _Vulkan
	{
		struct VertexDebug
		{
			static vk::VertexInputBindingDescription GetBindingDescription();
			static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions();
			glm::vec3 pos;
			glm::vec3 color;
		};
	}
}