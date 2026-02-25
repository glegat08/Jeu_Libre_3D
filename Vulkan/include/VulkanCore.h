#pragma once

#include <vector>

#include "Buffer.h"
#include "Global.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Surface.h"
#include "Device.h"
#include "ImagesViews.h"
#include "Pipeline.h"
#include "Queue.h"
#include "SwapChain.h"
#include "CommandBuffers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <stb_image.h>

#include "DescriptorLayout.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Image.h"
#include "SyncObject.h"


struct ImDrawData;

namespace KGR
{
	namespace _Vulkan
	{
		class VulkanCore
		{
		public:
			VulkanCore(GLFWwindow* window);
			void initVulkan();
			void mainLoop();
			void recreateSwapChain();
			std::uint32_t PresentImage();
			void recordCommandBuffer(uint32_t imageIndex, vk::raii::CommandBuffer& buffer);
			void recordCommandBuffer(uint32_t imageIndex, vk::raii::CommandBuffer& buffer, ImDrawData* drawData);
			void LoadModel();
			void transition_image_layout(
				vk::Image               image,
				vk::ImageLayout         old_layout,
				vk::ImageLayout         new_layout,
				vk::AccessFlags2        src_access_mask,
				vk::AccessFlags2        dst_access_mask,
				vk::PipelineStageFlags2 src_stage_mask,
				vk::PipelineStageFlags2 dst_stage_mask, vk::ImageAspectFlags    image_aspect_flags, vk::raii::CommandBuffer& buffer);

			void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,uint32_t mipLevels);

			void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


			void drawFrame();
			void drawFrame(ImDrawData* drawData);

			// to move 
			void	createTextureSampler();


			// never Use !!!
			static bool hasStencilComponent(vk::Format format);
			// find the depth format generic version
			void updateUniformBuffer(uint32_t currentImage);
			// callBack for instance
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);



			Instance& GetInstance();
			const Instance& GetInstance() const;

			Surface& GetSurface();
			const Surface& GetSurface() const;

			PhysicalDevice& GetPhysicalDevice();
			const PhysicalDevice& GetPhysicalDevice() const;

			Device& GetDevice();
			const Device& GetDevice() const;

			Queue& GetQueue();
			const Queue& GetQueue() const;

			SwapChain& GetSwapChain();
			const SwapChain& GetSwapChain() const;

			ImagesViews& GetImagesViews();
			const ImagesViews& GetImagesViews() const;

			Pipeline& GetGraphicsPipeline();
			const Pipeline& GetGraphicsPipeline() const;

			DescriptorPool& GetDescriptorPool();
			const DescriptorPool& GetDescriptorPool() const;

		private:
			// window
			GLFWwindow* window = nullptr;

			Instance               instance;
			Surface                surface;
			PhysicalDevice         physicalDevice ;
			Device				   device;
			Queue				   queue ;
			SwapChain              swapChain;
			ImagesViews            swapChainImageViews;

			DescriptorLayouts descriptorSetLayout;
			DescriptorPool descriptorPool;
			std::vector<DescriptorSet> descriptorSets;
			Pipeline               graphicsPipeline;

			Buffer vertexBuffer;
			Buffer indexBuffer;
			CommandBuffers         commandBuffers;


			std::vector<Buffer> uniformBuffers;

			SyncObject syncObject;


			Image textureImage;
			vk::raii::Sampler      textureSampler = nullptr;



			Image depthImage;

			std::vector<const char*> requiredDeviceExtension = {
				vk::KHRSwapchainExtensionName };

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
		};
	}
}
