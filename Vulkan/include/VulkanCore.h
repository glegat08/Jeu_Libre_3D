#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <optional>
#include <memory>
#include <algorithm>

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
#include "../../Editor/include/Offscreen.h"
#include "Core/Materials.h"

struct Texture;
struct CameraComponent;
class TransformComponent;
struct MeshComponent;

/**
 * @brief Struct containing mesh data for rendering.
 */
struct MeshData
{
	/**
	 * @brief Model transformation matrix.
	 */
	glm::mat4 matrixModel;

	/**
	 * @brief Pointer to mesh object.
	 */
	Mesh* mesh = nullptr;

	/**
	 * @brief Pointer to vector of textures.
	 */
	std::vector<Material> texture;

	int boneOffset = -1;
};

/**
 * @brief Struct representing a line segment for debug rendering.
 */
struct Segment
{
	glm::vec3 pos1;   ///< Start position of the segment
	glm::vec3 pos2;   ///< End position of the segment
	float  thickness; ///< Line thickness
	glm::vec4 color;  ///< Line color
};

struct StbiCleaner
{
	void operator()(stbi_uc* pixels) const
	{ 
		stbi_image_free(pixels);
	}
};

/*
* @brief Struct containing data for UI rendering.
*/
struct alignas(16) PushConstantData
{
	glm::mat4 model;
	int boneOffset;
	int pad1, pad2, pad3;
};

namespace KGR
{
	namespace _Vulkan
	{
		/**
		 * @brief Core Vulkan abstraction for rendering, resource management, and synchronization.
		 *
		 * Handles initialization of Vulkan, swap chain, images, pipelines, buffers, synchronization objects,
		 * and provides utilities to create vertex/index buffers, textures, and descriptor sets.
		 */
		class VulkanCore
		{
		public:

			/**
			 * @brief Initializes Vulkan and creates all required resources.
			 * @param window Pointer to GLFW window.
			 */
			void initVulkan(GLFWwindow* window);

			/**
			 * @brief Debug callback function for Vulkan validation layers.
			 */
			static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
				vk::DebugUtilsMessageTypeFlagsEXT type,
				const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void*);

			/**
			 * @brief Creates a texture sampler.
			 */
			void createTextureSampler();

			/**
			 * @brief Loads an image from file and creates a Vulkan image object.
			 * @param filePath Path to the image file.
			 * @return Vulkan Image object.
			 */
			Image CreateImage(const std::string& filePath);

			/**
			* @brief Creates a Vulkan image from encoded image data in memory (e.g., PNG, JPEG).
			* @param data Pointer to the encoded image bytes.
			* @param size Size of the encoded image data in bytes.
			* @return Vulkan Image object created from the decoded image data.
			*/
			Image CreateImageFromMemory(const unsigned char* data, size_t size);

			/**
			 * @brief creates a GPU image from a raw RGBA pixel buffer without stbi decoding.
			 * @param rgbaPixels pointer to contiguous RGBA bytes, 4 bytes per pixel.
			 * @param width image width in pixels.
			 * @param height image height in pixels.
			 * @return uploaded Vulkan image with view and mipmaps.
			 */
			Image CreateImageRaw(const uint8_t* rgbaPixels, uint32_t width, uint32_t height);

			/**
			 * @brief Creates a descriptor set for a given image.
			 * @param image Pointer to Vulkan Image.
			 * @return Descriptor set associated with the image.
			 */
			DescriptorSet CreateSetForImage(Image* image);

			/*DescriptorSet CreateSetForImageUi(Image* image);*/
			/**
			 * @brief Creates a vertex buffer from a vector of vertices.
			 * @tparam VertexT Type of vertex.
			 * @param vertices Vector of vertices.
			 * @return Vulkan Buffer containing vertex data.
			 */
			template<typename VertexT>
			Buffer CreateVertexBuffer(const std::vector<VertexT>& vertices);

			/**
			 * @brief Creates an index buffer from a vector of indices.
			 * @tparam IndexT Type of index.
			 * @param indices Vector of indices.
			 * @return Vulkan Buffer containing index data.
			 */
			template<typename IndexT>
			Buffer CreateIndexBuffer(const std::vector<IndexT>& indices);

			// Getters for Vulkan core resources
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

			DebugRenderer& GetDebugRenderer();
			const DebugRenderer& GetDebugRenderer() const;

			/**
			 * @brief Registers a light for rendering.
			 * @param light Light data to register.
			 */
			void RegisterLight(const LightData& light);

			/**
			 * @brief Registers camera matrices for rendering.
			 * @param model Model matrix
			 * @param view View matrix
			 * @param proj Projection matrix
			 */
			void RegisterCam(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);

			/**
			 * @brief Registers a mesh for rendering.
			 * @param mesh Mesh to render
			 * @param model Model transformation
			 * @param texture Vector of textures for the mesh
			 */
			void RegisterRender(Mesh& mesh, const glm::mat4& model, const  std::vector<Material>& texture, int boneOffset = -1);

			void RegisterUi(const UiData& data, Texture* texture,const glm::vec2& screenSize);
			/**
			 * @brief Performs rendering of registered meshes, lights, and optionally ImGui data.
			 * @param window GLFW window pointer
			 * @param clearColor Color to clear the screen
			 * @param imguiDraw Optional ImGui draw data
			 */
			void Render(GLFWwindow* window, const glm::vec4& clearColor = { 0,0,0,1 }, ImDrawData* imguiDraw = nullptr, KGR::Editor::Offscreen* offscreen = nullptr);

			/*
			* @brief Registers bone transformation matrices for skeletal animation.
			* @param boneMatrices Vector of bone transformation matrices to register.
			* @return Offset index in the bone buffer where the matrices were registered, or -1 on failure (e.g., if capacity is exceeded).
			*/
			int RegisterBoneMatrices(const std::vector<glm::mat4>& boneMatrices);

		private:

			int BeginRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, Pipeline* pipeline, const glm::vec4& color = { 0,0,0,1 });
			int EndRendering(GLFWwindow* window, vk::raii::CommandBuffer* currentBuffer, const std::vector<vk::Semaphore>& waitS, ImDrawData* imguiDraw = nullptr);
			void recreateSwapChain(GLFWwindow* window);
			std::uint32_t PresentImage();

			void RenderSceneToOffscreen(KGR::Editor::Offscreen& target, const glm::vec4& clearColor, vk::raii::CommandBuffer* cmd);

			void LoadModel();

			// Image layout and mipmap utilities
			void transition_image_layout(vk::Image image,
				vk::ImageLayout old_layout,
				vk::ImageLayout new_layout,
				vk::AccessFlags2 src_access_mask,
				vk::AccessFlags2 dst_access_mask,
				vk::PipelineStageFlags2 src_stage_mask,
				vk::PipelineStageFlags2 dst_stage_mask,
				vk::ImageAspectFlags image_aspect_flags,
				vk::raii::CommandBuffer& buffer);

			void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
			void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

			// Vulkan core resources
			Instance               instance;
			Surface                surface;
			PhysicalDevice         physicalDevice;
			Device                 device;
			Queue                  queue;
			SwapChain              swapChain;
			ImagesViews            swapChainImageViews;

			DescriptorLayouts descriptorSetLayout;
			DescriptorLayouts uiLayout;
			DescriptorPool descriptorPool;
			DescriptorSet descriptorSets;
			Pipeline               graphicsPipeline;
			Pipeline               linePipeLine;
			Pipeline               uiPipeline;

			DebugRenderer          debugRenderer;

			CommandBuffers commandBuffers;

			SyncObject syncObject;
			vk::raii::Sampler textureSampler = nullptr;
			Image depthImage;

			std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

			// Temporary buffers
			Buffer stagingVertexBuffer;
			Buffer vertexBuffer;
			Buffer stagingIndexBuffer;
			Buffer indexBuffer;
			Buffer m_boneBuffer;
			Buffer uniformBuffers;

			std::vector<glm::mat4> m_frameBones;
			std::vector<LightData> m_lights;
			DescriptorSet m_LightSet;
			Buffer m_lightBuffer;
			Buffer m_lightCount;
			std::optional<UniformBufferObject> m_ubo;
			std::vector<MeshData> m_toRenderObject;
			std::vector<std::pair<Texture*, UiData::UiValidData>> uIRender;



			Buffer uiVertexBuffer;
			Buffer uiIndexBuffer;

		};

		template <typename VertexT>
		Buffer VulkanCore::CreateVertexBuffer(const std::vector<VertexT>& vertices)
		{
			size_t vertSize = vertices.size() * sizeof(VertexT);
			auto vertexTmp = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertSize);
			vertexTmp.MapMemory(vertSize);
			vertexTmp.Upload(vertices);
			vertexTmp.UnMapMemory();
			auto vertexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vertSize);
			vertexBuffer.Copy(&vertexTmp, &device, &queue, &commandBuffers);
			return vertexBuffer;
		}

		template <typename IndexT>
		Buffer VulkanCore::CreateIndexBuffer(const std::vector<IndexT>& indices)
		{
			size_t indexSize = indices.size() * sizeof(IndexT);
			auto indexTmp = KGR::_Vulkan::Buffer(&device, &physicalDevice, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexSize);
			indexTmp.MapMemory(indexSize);
			indexTmp.Upload(indices);
			indexTmp.UnMapMemory();
			auto indexBuffer = KGR::_Vulkan::Buffer(&device, &physicalDevice,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal, indexSize);
			indexBuffer.Copy(&indexTmp, &device, &queue, &commandBuffers);
			return indexBuffer;
		}
	}
}