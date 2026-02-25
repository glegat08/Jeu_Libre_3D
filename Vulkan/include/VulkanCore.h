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
#include "RTTI.h"
#include "SyncObject.h"
#include "Core/Vertex.h"

struct CameraComponent;
class TransformComponent;
class MeshComponent;

struct ImDrawData;

namespace KGR
{
	namespace _Vulkan
	{
		class VulkanCore
		{
		public:
			using pair = std::pair<glm::mat4, MeshComponent*>;
			/**
			 * @brief 
			 * @param window 
			 */
			VulkanCore(GLFWwindow* window);
			void initVulkan();
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

		
			
		
			template<typename Type>
			Type& Get()
			{
				if constexpr (std::is_same_v<Type, Instance>)
					return instance;
				else if constexpr (std::is_same_v<Type, Surface>)
					return surface;
				else if constexpr (std::is_same_v<Type, PhysicalDevice>)
					return physicalDevice;
				else if constexpr (std::is_same_v<Type, Device>)
					return device;
				else if constexpr (std::is_same_v<Type, Queue>)
					return queue;
				else if constexpr (std::is_same_v<Type, SwapChain>)
					return swapChain;
				else if constexpr (std::is_same_v<Type, Pipeline>)
					return graphicsPipeline;
				else if constexpr (std::is_same_v<Type, CommandBuffers>)
					return commandBuffers;
				else static_assert("Type not supported");

			}
			// never Use !!!
			static bool hasStencilComponent(vk::Format format);
			// find the depth format generic version
			// callBack for instance
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);



			void RegisterCam(CameraComponent& cam, TransformComponent& transform);
			void RegisterRender(MeshComponent& mesh, TransformComponent& transform);
			void Render(const glm::vec4& color = { 0,0,0,1 });
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
			int BeginRendering(const glm::vec4& color = {0,0,0,1});
			int EndRendering();
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
			DescriptorSet descriptorSets;
			Pipeline               graphicsPipeline;

			Buffer vertexBuffer;
			Buffer indexBuffer;
			CommandBuffers         commandBuffers;


		Buffer uniformBuffers;

			SyncObject syncObject;


			Image textureImage;
			vk::raii::Sampler      textureSampler = nullptr;



			Image depthImage;

			std::vector<const char*> requiredDeviceExtension = {
				vk::KHRSwapchainExtensionName };

			//std::vector<Vertex> vertices;
			//std::vector<uint32_t> indices;

			vk::raii::CommandBuffer* m_currentBuffer;


			std::optional<UniformBufferObject> m_ubo;
			std::vector<pair> m_toRenderObject;
		};
	}
}
