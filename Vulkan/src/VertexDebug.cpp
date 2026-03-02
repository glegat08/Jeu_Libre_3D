#include "VertexDebug.h"

vk::VertexInputBindingDescription KGR::_Vulkan::VertexDebug::GetBindingDescription()
{
	return vk::VertexInputBindingDescription{
		0, sizeof(VertexDebug), vk::VertexInputRate::eVertex
	};
}

std::array<vk::VertexInputAttributeDescription, 2> KGR::_Vulkan::VertexDebug::GetAttributeDescriptions()
{
	return std::array<vk::VertexInputAttributeDescription, 2>{
		vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexDebug, pos) },
			vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexDebug, color) }
	};
}