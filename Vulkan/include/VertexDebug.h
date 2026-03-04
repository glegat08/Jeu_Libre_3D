#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace KGR
{
	namespace _Vulkan
	{
		/**
		 * @brief Structure representing a debug vertex with position and color.
		 *
		 * Provides helper functions to generate Vulkan vertex input binding and attribute descriptions
		 * for use in debug rendering pipelines.
		 */
		struct VertexDebug
		{
			/**
			 * @brief Returns the Vulkan vertex input binding description for this vertex type.
			 * @return Vulkan vertex input binding description.
			 */
			static vk::VertexInputBindingDescription GetBindingDescription();

			/**
			 * @brief Returns Vulkan vertex input attribute descriptions for position and color.
			 * @return Array of Vulkan vertex input attribute descriptions.
			 */
			static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions();

			/**
			 * @brief Vertex position in 3D space.
			 */
			glm::vec3 pos;

			/**
			 * @brief Vertex color.
			 */
			glm::vec3 color;
		};
	}
}