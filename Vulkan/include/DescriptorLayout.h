#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Pipeline;
		class Device;  ///< Forward declaration of Vulkan logical device

		/**
		 * @brief Wrapper for a Vulkan descriptor set layout using RAII.
		 *
		 * Encapsulates creation, management, and access of a Vulkan descriptor
		 * set layout. Provides RAII semantics for automatic cleanup.
		 */
		class DescriptorLayout
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII descriptor set layout.
			 */
			using vkDescriptorLayout = vk::raii::DescriptorSetLayout;

			/**
			 * @brief Alias for Vulkan descriptor set layout binding.
			 */
			using vkBinding = vk::DescriptorSetLayoutBinding;

			/**
			 * @brief Default constructor.
			 */
			DescriptorLayout() = default;

			/**
			 * @brief Constructs a descriptor layout with specified bindings.
			 *
			 * @param bindings Vector of Vulkan descriptor set layout bindings.
			 * @param device Pointer to the Vulkan logical device.
			 */
			DescriptorLayout(const std::vector<vkBinding>& bindings, Device* device);

			/**
			 * @brief Returns the underlying Vulkan descriptor set layout.
			 * @return Reference to the RAII Vulkan descriptor set layout.
			 */
			vkDescriptorLayout& Get();

			/**
			 * @brief Returns the underlying Vulkan descriptor set layout (const).
			 * @return Const reference to the RAII Vulkan descriptor set layout.
			 */
			const vkDescriptorLayout& Get() const;

		private:

			/**
			 * @brief RAII Vulkan descriptor set layout handle.
			 */
			vkDescriptorLayout m_layout = nullptr;
		};

		/**
		 * @brief Container for multiple Vulkan descriptor layouts.
		 *
		 * Allows adding, accessing, and retrieving multiple descriptor layouts,
		 * including their raw Vulkan handles for pipeline creation.
		 */
		class DescriptorLayouts
		{
		public:

			/**
			 * @brief Pipeline class is a friend to access internal layouts directly.
			 */
			friend Pipeline;

			/**
			 * @brief Default constructor.
			 */
			DescriptorLayouts() = default;

			/**
			 * @brief Adds a descriptor layout to the container.
			 *
			 * @param layout DescriptorLayout object to add (rvalue reference).
			 */
			void Add(DescriptorLayout&& layout);

			/**
			 * @brief Retrieves a descriptor layout by index.
			 *
			 * @param index Index of the descriptor layout to retrieve.
			 * @return Reference to the descriptor layout.
			 */
			DescriptorLayout& Get(size_t index);

			/**
			 * @brief Retrieves a descriptor layout by index (const version).
			 *
			 * @param index Index of the descriptor layout to retrieve.
			 * @return Const reference to the descriptor layout.
			 */
			const DescriptorLayout& Get(size_t index) const;

			/**
			 * @brief Returns the number of descriptor layouts in the container.
			 * @return Number of descriptor layouts stored.
			 */
			size_t Size();

		private:

			/**
			 * @brief Retrieves raw Vulkan descriptor set layout pointers.
			 *
			 * @return Vector of pointers to Vulkan descriptor set layouts.
			 */
			std::vector<DescriptorLayout::vkDescriptorLayout*> GetLayouts();

			/**
			 * @brief Container holding DescriptorLayout objects.
			 */
			std::vector<DescriptorLayout> m_layouts;

			/**
			 * @brief Container holding raw Vulkan descriptor layout pointers.
			 */
			std::vector<DescriptorLayout::vkDescriptorLayout*> m_realLayouts;
		};
	}
}