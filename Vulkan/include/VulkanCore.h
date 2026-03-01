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
#include "Core/LightComponent.h"
#include "Core/TrasformComponent.h"
#include "Core/Vertex.h"

struct TextureComponent;
struct CameraComponent;
class TransformComponent;
struct MeshComponent;

struct MeshData
{
	glm::mat4 matrixModel; 
	MeshComponent* mesh = nullptr;
	TextureComponent* texture = nullptr;
};

struct ImDrawData;

namespace KGR
{
	namespace _Vulkan
	{





		class VulkanCore
		{
		public:
			void initVulkan(GLFWwindow* window);
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);
			void createTextureSampler();
			Image CreateImage(const std::string& filePath);
			DescriptorSet CreateSetForImage(Image* image);
			template<typename VertexT>
			Buffer CreateVertexBuffer(const std::vector<VertexT>& vertices);
			template<typename IndexT>
			Buffer CreateIndexBuffer(const std::vector<IndexT>& indices);
			
			template<LightData::Type Type>
			void RegisterLight(LightComponent<Type>& light, TransformComponent& transform);
			void RegisterCam(CameraComponent& cam, TransformComponent& transform);
			void RegisterRender(MeshComponent& mesh, TransformComponent& transform,TextureComponent& texture);
			void Render(GLFWwindow* window,const glm::vec4& clearColor = { 0,0,0,1 });
		private:
			int BeginRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, const glm::vec4& color = {0,0,0,1});
			int EndRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer);
			void recreateSwapChain(GLFWwindow* window);
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
			void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
			void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
/*

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
			void Render(const glm::vec4& color = { 0,0,0,1 }, ImDrawData* drawData = nullptr);

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
			int EndRendering(ImDrawData* drawData = nullptr);

			// window
			GLFWwindow* window = nullptr;
*/
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
			CommandBuffers         commandBuffers;
			
			SyncObject syncObject;
			vk::raii::Sampler textureSampler = nullptr;
			Image depthImage;
			std::vector<const char*> requiredDeviceExtension = {
				vk::KHRSwapchainExtensionName };

  /*
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			vk::raii::CommandBuffer* m_currentBuffer;
*/

			Buffer uniformBuffers;
			std::vector<LightData> m_lights;
			DescriptorSet m_LightSet;
			Buffer m_lightBuffer;
			Buffer m_lightCount;
			std::optional<UniformBufferObject> m_ubo;
			std::vector<MeshData> m_toRenderObject;
		};

		template <typename VertexT>
		Buffer VulkanCore::CreateVertexBuffer(const std::vector<VertexT>& vertices)
		{
			size_t vertSize = vertices.size() * sizeof(VertexT);
			auto vertexTmp = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertSize);
			vertexTmp.MapMemory(vertSize);
			vertexTmp.Upload(vertices);
			vertexTmp.UnMapMemory();
			auto vertexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertSize);
			vertexBuffer.Copy(&vertexTmp, &device, &queue, &commandBuffers);
			return vertexBuffer;
		}

		template <typename IndexT>
		Buffer VulkanCore::CreateIndexBuffer(const std::vector<IndexT>& indices)
		{
			size_t indexSize = indices.size() * sizeof(IndexT);
			auto indexTmp = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexSize);
			indexTmp.MapMemory(indexSize);
			indexTmp.Upload(indices);
			indexTmp.UnMapMemory();
			auto indexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexSize);
			indexBuffer.Copy(&indexTmp, &device, &queue, &commandBuffers);
			return indexBuffer;
		}

		template <LightData::Type Type>
		void VulkanCore::RegisterLight(LightComponent<Type>& light, TransformComponent& transform)
		{
			LightData lightData = light.ToData();
			if constexpr (Type == LightData::Type::Point)
				lightData.pos = transform.GetPosition();
			else if constexpr (Type == LightData::Type::Directional)
			{
				lightData.dir = transform.GetLocalAxe<RotData::Dir::Forward>();
			}
			else
			{
				lightData.pos = transform.GetPosition();
				lightData.dir = transform.GetLocalAxe<RotData::Dir::Forward>();
			}
			m_lights.push_back(lightData);
		}
	}
}
