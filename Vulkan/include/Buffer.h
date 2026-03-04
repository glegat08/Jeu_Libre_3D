#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class CommandBuffers;
		class Device;
		class Queue;
		class PhysicalDevice;
		class Image;

		/**
		 * @brief Vulkan buffer wrapper using RAII.
		 *
		 * This class encapsulates a Vulkan buffer and its associated device memory.
		 * It provides utilities for buffer creation, memory mapping, data upload,
		 * and copy operations between buffers and images.
		 */
		class Buffer
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII buffer type.
			 */
			using vkBuffer = vk::raii::Buffer;

			/**
			 * @brief Alias for Vulkan RAII device memory type.
			 */
			using vkBufferMemory = vk::raii::DeviceMemory;

			/**
			 * @brief Default constructor.
			 */
			Buffer() = default;

			/**
			 * @brief Returns the underlying Vulkan buffer.
			 * @return Reference to the Vulkan RAII buffer.
			 */
			vkBuffer& Get();

			/**
			 * @brief Returns the underlying Vulkan buffer (const).
			 * @return Const reference to the Vulkan RAII buffer.
			 */
			const vkBuffer& Get() const;

			/**
			 * @brief Constructs and creates a Vulkan buffer.
			 *
			 * @param device Pointer to the logical device.
			 * @param phDevice Pointer to the physical device.
			 * @param usage Buffer usage flags.
			 * @param MemoryProperties Memory property flags.
			 * @param size Size of the buffer in bytes.
			 */
			Buffer(Device* device, PhysicalDevice* phDevice, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags MemoryProperties, size_t size);

			/**
			 * @brief Uploads data from a std::array to the buffer.
			 *
			 * @tparam elemType Type of the array elements.
			 * @tparam elemSize Number of elements in the array.
			 * @param data Array containing the data to upload.
			 */
			template<typename elemType, size_t elemSize>
			void Upload(const std::array<elemType, elemSize>& data);

			/**
			 * @brief Uploads data from a std::vector to the buffer.
			 *
			 * @tparam elemType Type of the vector elements.
			 * @param data Vector containing the data to upload.
			 */
			template<typename elemType>
			void Upload(const std::vector<elemType>& data);

			/**
			 * @brief Uploads raw data to the buffer.
			 *
			 * @param data Pointer to the source data.
			 * @param size Size of the data in bytes.
			 */
			void Upload(const void* data, size_t size);

			/**
			 * @brief Copies the content of another buffer into this buffer.
			 *
			 * @param other Source buffer.
			 * @param device Pointer to the logical device.
			 * @param queue Pointer to the Vulkan queue.
			 * @param buffers Command buffers used for the copy operation.
			 */
			void Copy(Buffer* other, Device* device, Queue* queue, CommandBuffers* buffers);

			/**
			 * @brief Copies the content of another buffer asynchronously.
			 *
			 * @param other Source buffer.
			 * @param device Pointer to the logical device.
			 * @param queue Pointer to the Vulkan queue.
			 * @param buffers Command buffers used for the copy operation.
			 * @return Vulkan semaphore used for synchronization.
			 */
			vk::raii::Semaphore CopyAssync(Buffer* other, Device* device, Queue* queue, CommandBuffers* buffers);

			/**
			 * @brief Copies the buffer content into a Vulkan image.
			 *
			 * @param image Destination image.
			 * @param device Pointer to the logical device.
			 * @param queue Pointer to the Vulkan queue.
			 * @param buffers Command buffers used for the copy operation.
			 */
			void CopyImage(Image* image, Device* device, Queue* queue, CommandBuffers* buffers);

			/**
			 * @brief Creates a Vulkan buffer and allocates its memory.
			 *
			 * @param size Size of the buffer in bytes.
			 * @param usage Buffer usage flags.
			 * @param properties Memory property flags.
			 * @param buffer Reference to the Vulkan RAII buffer to create.
			 * @param bufferMemory Reference to the Vulkan RAII device memory to allocate.
			 * @param device Pointer to the logical device.
			 * @param phDevice Pointer to the physical device.
			 */
			static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
				vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory, Device* device, PhysicalDevice* phDevice);

			/**
			 * @brief Copies data from one Vulkan buffer to another.
			 *
			 * @param srcBuffer Source Vulkan buffer.
			 * @param dstBuffer Destination Vulkan buffer.
			 * @param size Size of the data to copy in bytes.
			 * @param device Pointer to the logical device.
			 * @param queue Pointer to the Vulkan queue.
			 * @param commandBuffer Command buffer used for the copy operation.
			 */
			static void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size, Device* device, Queue* queue, CommandBuffers* commandBuffer);

			/**
			 * @brief Copies data from one Vulkan buffer to another asynchronously.
			 *
			 * @param srcBuffer Source Vulkan buffer.
			 * @param dstBuffer Destination Vulkan buffer.
			 * @param size Size of the data to copy in bytes.
			 * @param device Pointer to the logical device.
			 * @param queue Pointer to the Vulkan queue.
			 * @param commandBuffer Command buffer used for the copy operation.
			 * @return Vulkan semaphore used for synchronization.
			 */
			static vk::raii::Semaphore copyBufferAssync(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size, Device* device, Queue* queue, CommandBuffers* commandBuffer);

			/**
			 * @brief Finds a suitable memory type on the physical device.
			 *
			 * @param typeFilter Bitmask of suitable memory types.
			 * @param properties Required memory property flags.
			 * @param phDevice Pointer to the physical device.
			 * @return Index of the suitable memory type.
			 */
			static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, PhysicalDevice* phDevice);

			/**
			 * @brief Maps the buffer memory.
			 *
			 * @param size Size of the memory region to map.
			 */
			void MapMemory(size_t size);

			/**
			 * @brief Unmaps the buffer memory.
			 */
			void UnMapMemory();

			/**
			 * @brief Returns the size of the buffer.
			 * @return Buffer size in bytes.
			 */
			size_t GetSize() const;

		private:

			/**
			 * @brief Pointer to mapped memory destination.
			 */
			void* dest = nullptr;

			/**
			 * @brief Size of the buffer in bytes.
			 */
			size_t m_size = 0;

			/**
			 * @brief Vulkan RAII buffer handle.
			 */
			vkBuffer m_buffer = nullptr;

			/**
			 * @brief Vulkan RAII device memory handle.
			 */
			vkBufferMemory m_bufferMemory = nullptr;
		};

		template <typename elemType, size_t elemSize>
		void Buffer::Upload(const std::array<elemType, elemSize>& data)
		{
			Upload(data.data(), data.size() * (data.empty() ? 0 : sizeof(data[0])));
		}

		template <typename elemType>
		void Buffer::Upload(const std::vector<elemType>& data)
		{
			Upload(data.data(), data.size() * (data.empty() ? 0 : sizeof(data[0])));
		}

	}
}