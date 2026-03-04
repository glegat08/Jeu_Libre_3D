#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class PhysicalDevice; ///< Forward declaration of Vulkan physical device
		class Surface;        ///< Forward declaration of Vulkan surface

		/**
		 * @brief Wrapper for a Vulkan logical device using RAII.
		 *
		 * This class encapsulates the creation and management of a Vulkan logical device,
		 * including queue selection and RAII handle management.
		 */
		class Device
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII device type.
			 */
			using vkDevice = vk::raii::Device;

			/**
			 * @brief Default constructor.
			 */
			Device() = default;

			/**
			 * @brief Constructs a Vulkan logical device from a physical device and surface.
			 *
			 * @param device Pointer to the Vulkan physical device.
			 * @param surface Pointer to the Vulkan surface.
			 * @param count Number of queues to create (default is 1).
			 */
			Device(PhysicalDevice* device, Surface* surface, ui32t count = 1);

			/**
			 * @brief Returns the underlying Vulkan RAII device.
			 * @return Reference to the Vulkan RAII device.
			 */
			vkDevice& Get();

			/**
			 * @brief Returns the underlying Vulkan RAII device (const).
			 * @return Const reference to the Vulkan RAII device.
			 */
			const vkDevice& Get() const;

			/**
			 * @brief Returns the index of the Vulkan queue.
			 * @return Queue index.
			 */
			uint32_t GetQueueIndex() const
			{
				return m_queueIndex;
			}

			/**
			 * @brief Returns the number of queues created in the device.
			 * @return Queue count.
			 */
			uint32_t GetQueueCount() const
			{
				return m_queueCount;
			}

		private:

			/**
			 * @brief RAII Vulkan logical device handle.
			 */
			vkDevice m_device = nullptr;

			/**
			 * @brief Index of the queue used by this device.
			 */
			uint32_t m_queueIndex;

			/**
			 * @brief Number of queues created for this device.
			 */
			uint32_t m_queueCount;
		};
	}
}