#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Device;           ///< Forward declaration of Vulkan logical device
		class DescriptorLayout; ///< Forward declaration of descriptor layout
		class DescriptorPool;   ///< Forward declaration of descriptor pool

		/**
		 * @brief Wrapper for a Vulkan descriptor set using RAII.
		 *
		 * Encapsulates creation, access, and management of a Vulkan descriptor set.
		 * Provides utility functions to create multiple sets at once.
		 */
		class DescriptorSet
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII descriptor set.
			 */
			using vkDescriptorSet = vk::raii::DescriptorSet;

			/**
			 * @brief Alias for Vulkan write descriptor set.
			 */
			using vkWriteSet = vk::WriteDescriptorSet;

			/**
			 * @brief Default constructor.
			 */
			DescriptorSet() = default;

			/**
			 * @brief Constructs a descriptor set with a device, pool, and layout.
			 *
			 * @param device Pointer to the Vulkan logical device.
			 * @param pool Pointer to the Vulkan descriptor pool.
			 * @param layout Pointer to the descriptor layout.
			 */
			DescriptorSet(Device* device, DescriptorPool* pool, DescriptorLayout* layout);

			/**
			 * @brief Creates multiple descriptor sets in a single call.
			 *
			 * @param device Pointer to the Vulkan logical device.
			 * @param pool Pointer to the Vulkan descriptor pool.
			 * @param layout Pointer to the descriptor layout.
			 * @param count Number of descriptor sets to create.
			 * @return Vector containing the created descriptor sets.
			 */
			static std::vector<DescriptorSet> Create(Device* device, DescriptorPool* pool, DescriptorLayout* layout, size_t count);

			/**
			 * @brief Returns the underlying Vulkan descriptor set.
			 * @return Reference to the Vulkan RAII descriptor set.
			 */
			vkDescriptorSet& Get()
			{
				return m_set;
			}

			/**
			 * @brief Returns the underlying Vulkan descriptor set (const).
			 * @return Const reference to the Vulkan RAII descriptor set.
			 */
			const vkDescriptorSet& Get() const
			{
				return m_set;
			}

		private:

			/**
			 * @brief Private constructor from an existing Vulkan descriptor set.
			 *
			 * @param set Vulkan RAII descriptor set to wrap.
			 */
			DescriptorSet(vkDescriptorSet&& set) : m_set(std::move(set)) {}

			/**
			 * @brief RAII Vulkan descriptor set handle.
			 */
			vkDescriptorSet m_set = nullptr;
		};
	}
}