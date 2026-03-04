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
#include "DebugRenderer.h"
#include "VertexDebug.h"
#include "Core/Texture.h"
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
#include "../../ImGui/include/imgui.h"
#include "Core/LightComponent.h"

#include "Core/TrasformComponent.h"
#include "Core/Vertex.h"

struct Texture;
struct CameraComponent;
class TransformComponent;
struct MeshComponent;

struct MeshData
{
	glm::mat4 matrixModel;
	Mesh* mesh = nullptr;
	std::vector<Texture*>* texture;
};

struct ImDrawData;

struct Segment
{
	glm::vec3 pos1;
	glm::vec3 pos2;
	float  thickness;
	glm::vec4 color;
};

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

			void RegisterLight(const LightData& light);
			void RegisterCam(const glm::mat4& model,const glm::mat4& view , const glm::mat4& proj);
			void RegisterRender(Mesh& mesh,const  glm::mat4& model,std::vector<Texture*>& texture);
			void Render(GLFWwindow* window,const glm::vec4& clearColor = { 0,0,0,1 }, ImDrawData* imguiDraw = nullptr);
		
      private:
			int BeginRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, Pipeline* pipeline, const glm::vec4& color = {0,0,0,1});
			int EndRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer,const std::vector<vk::Semaphore>& waitS, ImDrawData* imguiDraw = nullptr);
			void recreateSwapChain(GLFWwindow* window);
			std::uint32_t PresentImage();

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



			Instance               instance;
			Surface                surface;
			PhysicalDevice         physicalDevice;
			Device				   device;
			Queue				   queue;
			SwapChain              swapChain;
			ImagesViews            swapChainImageViews;

			DescriptorLayouts descriptorSetLayout;
			DescriptorPool descriptorPool;
			DescriptorSet descriptorSets;
			Pipeline               graphicsPipeline;
			Pipeline               linePipeLine;

			CommandBuffers         commandBuffers;

			SyncObject syncObject;
			vk::raii::Sampler textureSampler = nullptr;
			Image depthImage;
			std::vector<const char*> requiredDeviceExtension = {
				vk::KHRSwapchainExtensionName };

			//tmp
			Buffer stagingVertexBuffer;
			Buffer vertexBuffer;
			Buffer stagingIndexBuffer;
			Buffer indexBuffer;




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

	}
}