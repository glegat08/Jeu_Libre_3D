#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Device;
		class SwapChain;

		/**
		 * @brief Wrapper for Vulkan command buffers and command pool management.
		 *
		 * This class encapsulates the creation, acquisition, and release of Vulkan
		 * command buffers. It also tracks GPU/CPU usage with fences.
		 */
		class CommandBuffers
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII command pool.
			 */
			using vkCommandPool = vk::raii::CommandPool;

			/**
			 * @brief Alias for Vulkan RAII command buffer.
			 */
			using vkCommandBuffer = vk::raii::CommandBuffer;

			/**
			 * @brief Alias for Vulkan RAII fence.
			 */
			using vkFence = vk::raii::Fence;

			/**
			 * @brief Structure representing a tracked command buffer with synchronization.
			 */
			struct KGRCommandBuffer
			{
				/**
				 * @brief The Vulkan command buffer handle.
				 */
				vkCommandBuffer commandBuffer = nullptr;

				/**
				 * @brief Fence to indicate if GPU work is finished.
				 */
				vkFence     isGpuFree = nullptr;

				/**
				 * @brief Indicates whether the command buffer is available for CPU usage.
				 */
				bool        isCpuFree;
			};

			/**
			 * @brief Default constructor.
			 */
			CommandBuffers() = default;

			/**
			 * @brief Constructor that initializes the command pool for a device.
			 *
			 * @param device Pointer to the logical device.
			 */
			CommandBuffers(Device* device);

			/**
			 * @brief Returns the underlying Vulkan command pool.
			 * @return Reference to the Vulkan RAII command pool.
			 */
			vkCommandPool& GetPool();

			/**
			 * @brief Returns the underlying Vulkan command pool (const).
			 * @return Const reference to the Vulkan RAII command pool.
			 */
			const vkCommandPool& GetPool() const;

			/**
			 * @brief Acquires a free command buffer from the pool.
			 *
			 * @param device Pointer to the logical device.
			 * @return Reference to the acquired Vulkan command buffer.
			 */
			vkCommandBuffer& Acquire(Device* device);

			/**
			 * @brief Retrieves the fence associated with a command buffer.
			 *
			 * @param buffer Vulkan command buffer.
			 * @return Reference to the Vulkan fence used for synchronization.
			 */
			vkFence& GetFence(vkCommandBuffer& buffer);

			/**
			 * @brief Releases a command buffer back to the pool.
			 *
			 * @param commandBuffer Vulkan command buffer to release.
			 */
			void ReleaseCommandBuffer(vkCommandBuffer& commandBuffer);

		private:

			/**
			 * @brief Vulkan command pool handle.
			 */
			vkCommandPool m_pool = nullptr;

			/**
			 * @brief Map of Vulkan command buffers to tracked KGRCommandBuffer structures.
			 */
			std::unordered_map<VkCommandBuffer, KGRCommandBuffer> m_commandBuffers;
		};
	}
}