#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Device;  ///< Forward declaration of Vulkan logical device

		/**
		 * @brief Wrapper for a Vulkan descriptor pool using RAII.
		 *
		 * Manages creation and lifetime of a Vulkan descriptor pool and provides
		 * access to the underlying RAII handle.
		 */
		class DescriptorPool
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII descriptor pool.
			 */
			using vkPool = vk::raii::DescriptorPool;

			/**
			 * @brief Alias for Vulkan descriptor pool size structure.
			 */
			using vkPoolSize = vk::DescriptorPoolSize;

			/**
			 * @brief Default constructor.
			 */
			DescriptorPool() = default;

			/**
			 * @brief Constructs a Vulkan descriptor pool with given sizes and maximum count.
			 *
			 * @param info Vector of descriptor pool sizes.
			 * @param maxCount Maximum number of descriptors in the pool.
			 * @param device Pointer to the Vulkan logical device.
			 */
			DescriptorPool(const std::vector<vkPoolSize>& info, size_t maxCount, Device* device);

			/**
			 * @brief Returns the underlying Vulkan descriptor pool.
			 * @return Reference to the Vulkan RAII descriptor pool.
			 */
			vkPool& Get()
			{
				return m_pool;
			}

			/**
			 * @brief Returns the underlying Vulkan descriptor pool (const).
			 * @return Const reference to the Vulkan RAII descriptor pool.
			 */
			const vkPool& Get() const
			{
				return m_pool;
			}

		private:

			/**
			 * @brief RAII Vulkan descriptor pool handle.
			 */
			vkPool m_pool = nullptr;
		};
	}
}