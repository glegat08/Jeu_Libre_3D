#pragma once
#include "Global.h"
namespace KGR
{
	namespace _Vulkan
	{
		class PhysicalDevice;
		class DescriptorLayouts;

		struct ShaderInfo
		{
			const char* ShaderPath;
			const char* vertexMain;
			const char* fragmentMain;
		};

		class Device;
		class SwapChain;
		class DescriptorLayout;
		class Pipeline
		{
		public:
			using vkPipeline = vk::raii::Pipeline;
			using vkPipelineLayout = vk::raii::PipelineLayout;
			using vkDescriptorLayout = vk::raii::DescriptorSetLayout;

			Pipeline() = default;
			Pipeline(const ShaderInfo& shaderInfo,Device* device,SwapChain* swapChain, DescriptorLayouts* layouts,PhysicalDevice* phDevice, vk::PolygonMode mode,const vk::VertexInputBindingDescription& vInput,const std::vector < vk::VertexInputAttributeDescription>& attributes)
			;

			vkPipelineLayout& GetLayout();

			const vkPipelineLayout& GetLayout() const;

			vkPipeline& Get();

			const vkPipeline& Get() const;

		private:
			vkPipelineLayout m_layout = nullptr;
			vkPipeline m_pipeline = nullptr;
			std::vector<vk::DescriptorSetLayout> m_layouts;
		};
	}
}