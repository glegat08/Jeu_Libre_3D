#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class PhysicalDevice;    ///< Forward declaration of Vulkan physical device
		class Device;            ///< Forward declaration of Vulkan logical device
		class SwapChain;         ///< Forward declaration of Vulkan swap chain
		class DescriptorLayouts;  ///< Forward declaration of descriptor layouts
		class DescriptorLayout;  ///< Forward declaration of descriptor layout

		/**
		 * @brief Structure containing shader information for pipeline creation.
		 */
		struct ShaderInfo
		{
			/**
			 * @brief Path to the shader file.
			 */
			const char* ShaderPath;

			/**
			 * @brief Name of the main function in the vertex shader.
			 */
			const char* vertexMain;

			/**
			 * @brief Name of the main function in the fragment shader.
			 */
			const char* fragmentMain;
		};

		/**
		 * @brief Wrapper for a Vulkan graphics pipeline and layout.
		 *
		 * This class encapsulates pipeline creation, pipeline layout management,
		 * and provides access to underlying Vulkan RAII handles.
		 */
		class Pipeline
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII pipeline.
			 */
			using vkPipeline = vk::raii::Pipeline;

			/**
			 * @brief Alias for Vulkan RAII pipeline layout.
			 */
			using vkPipelineLayout = vk::raii::PipelineLayout;

			/**
			 * @brief Alias for Vulkan RAII descriptor set layout.
			 */
			using vkDescriptorLayout = vk::raii::DescriptorSetLayout;

			/**
			 * @brief Default constructor.
			 */
			Pipeline() = default;

			/**
			 * @brief Constructs a Vulkan graphics pipeline with shaders, device, swap chain, and descriptor layouts.
			 *
			 * @param shaderInfo Shader information including file paths and entry points.
			 * @param device Pointer to the Vulkan logical device.
			 * @param swapChain Pointer to the Vulkan swap chain.
			 * @param layouts Pointer to the descriptor layouts container.
			 * @param phDevice Pointer to the Vulkan physical device.
			 * @param mode Polygon rasterization mode.
			 * @param vInput Vertex input binding description.
			 * @param attributes Vector of vertex input attribute descriptions.
			 */
			Pipeline(const ShaderInfo& shaderInfo, Device* device, SwapChain* swapChain, DescriptorLayouts* layouts, PhysicalDevice* phDevice,
				vk::PolygonMode mode, const vk::VertexInputBindingDescription& vInput,
				const std::vector<vk::VertexInputAttributeDescription>& attributes);

			/**
			 * @brief Returns the pipeline layout.
			 * @return Reference to the Vulkan RAII pipeline layout.
			 */
			vkPipelineLayout& GetLayout();

			/**
			 * @brief Returns the pipeline layout (const).
			 * @return Const reference to the Vulkan RAII pipeline layout.
			 */
			const vkPipelineLayout& GetLayout() const;

			/**
			 * @brief Returns the graphics pipeline.
			 * @return Reference to the Vulkan RAII pipeline.
			 */
			vkPipeline& Get();

			/**
			 * @brief Returns the graphics pipeline (const).
			 * @return Const reference to the Vulkan RAII pipeline.
			 */
			const vkPipeline& Get() const;

		private:

			/**
			 * @brief RAII Vulkan pipeline layout handle.
			 */
			vkPipelineLayout m_layout = nullptr;

			/**
			 * @brief RAII Vulkan graphics pipeline handle.
			 */
			vkPipeline m_pipeline = nullptr;

			/**
			 * @brief Vector storing Vulkan descriptor set layouts used by the pipeline.
			 */
			std::vector<vk::DescriptorSetLayout> m_layouts;
		};
	}
}