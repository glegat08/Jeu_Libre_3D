#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Device; ///< Forward declaration of Vulkan logical device

		/**
		 * @brief Wrapper for a Vulkan queue using RAII.
		 *
		 * Encapsulates access and management of a Vulkan queue retrieved from a logical device.
		 */
		class Queue
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII queue.
			 */
			using vkQueue = vk::raii::Queue;

			/**
			 * @brief Default constructor.
			 */
			Queue() = default;

			/**
			 * @brief Constructs a Vulkan queue from a logical device.
			 *
			 * @param device Pointer to the Vulkan logical device.
			 * @param id Queue index (default 0).
			 */
			Queue(Device* device, uint32_t id = 0);

			/**
			 * @brief Returns the Vulkan queue.
			 * @return Reference to the RAII Vulkan queue.
			 */
			vkQueue& Get();

			/**
			 * @brief Returns the Vulkan queue (const).
			 * @return Const reference to the RAII Vulkan queue.
			 */
			const vkQueue& Get() const;

		private:

			/**
			 * @brief RAII Vulkan queue handle.
			 */
			vkQueue m_queue = nullptr;
		};
	}
}