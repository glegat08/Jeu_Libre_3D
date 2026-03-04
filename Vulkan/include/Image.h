#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Device;           ///< Forward declaration of Vulkan logical device
		class PhysicalDevice;   ///< Forward declaration of Vulkan physical device

		/**
		 * @brief Wrapper for a Vulkan image with RAII management.
		 *
		 * This class handles creation, memory allocation, and view creation
		 * for Vulkan images. Provides utilities to access image properties
		 * and the underlying Vulkan handles.
		 */
		class Image
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII image type.
			 */
			using vkImage = vk::raii::Image;

			/**
			 * @brief Alias for Vulkan RAII image view type.
			 */
			using vkImageView = vk::raii::ImageView;

			/**
			 * @brief Alias for Vulkan RAII device memory type.
			 */
			using vkDeviceMemory = vk::raii::DeviceMemory;

			/**
			 * @brief Default constructor.
			 */
			Image() = default;

			/**
			 * @brief Constructs a Vulkan image with given parameters.
			 *
			 * @param width Image width in pixels.
			 * @param height Image height in pixels.
			 * @param mipLevel Number of mipmap levels.
			 * @param format Vulkan image format.
			 * @param tiling Vulkan image tiling.
			 * @param usage Vulkan image usage flags.
			 * @param properties Memory property flags.
			 * @param device Pointer to the logical device.
			 * @param physicalDevice Pointer to the physical device.
			 */
			Image(uint32_t width, uint32_t height, uint32_t mipLevel, vk::Format format, vk::ImageTiling tiling,
				vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, Device* device, PhysicalDevice* physicalDevice);

			/**
			 * @brief Creates a Vulkan image view for this image.
			 *
			 * @param format Vulkan image format.
			 * @param aspectFlags Image aspect flags.
			 * @param device Pointer to the logical device.
			 */
			void CreateView(vk::Format format, vk::ImageAspectFlags aspectFlags, Device* device);

			/**
			 * @brief Returns the Vulkan image view.
			 * @return Reference to the Vulkan RAII image view.
			 */
			vkImageView& GetView();

			/**
			 * @brief Returns the Vulkan image view (const).
			 * @return Const reference to the Vulkan RAII image view.
			 */
			const vkImageView& GetView() const;

			/**
			 * @brief Returns the Vulkan image.
			 * @return Reference to the Vulkan RAII image.
			 */
			vkImage& Get();

			/**
			 * @brief Returns the Vulkan image (const).
			 * @return Const reference to the Vulkan RAII image.
			 */
			const vkImage& Get() const;

			/**
			 * @brief Static helper to create a Vulkan image with memory allocation.
			 *
			 * @param width Image width.
			 * @param height Image height.
			 * @param mimMapLevel Number of mipmap levels.
			 * @param format Vulkan image format.
			 * @param tiling Vulkan image tiling.
			 * @param usage Vulkan image usage flags.
			 * @param properties Memory property flags.
			 * @param image Reference to store the created Vulkan image.
			 * @param imageMemory Reference to store allocated device memory.
			 * @param device Pointer to the logical device.
			 * @param physicalDevice Pointer to the physical device.
			 */
			static void createImage(uint32_t width, uint32_t height, uint32_t mimMapLevel, vk::Format format, vk::ImageTiling tiling,
				vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image,
				vk::raii::DeviceMemory& imageMemory, Device* device, PhysicalDevice* physicalDevice);

			/**
			 * @brief Static helper to create a Vulkan image view.
			 *
			 * @param image Vulkan image.
			 * @param format Vulkan image format.
			 * @param aspectFlags Image aspect flags.
			 * @param mipMapLevel Number of mipmap levels.
			 * @param device Pointer to the logical device.
			 * @return Created Vulkan image view.
			 */
			static vkImageView createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipMapLevel, Device* device);

			/**
			 * @brief Returns the image height.
			 * @return Image height in pixels.
			 */
			uint32_t GetHeight() const
			{
				return m_height;
			}

			/**
			 * @brief Returns the image width.
			 * @return Image width in pixels.
			 */
			uint32_t GetWidth() const
			{
				return m_width;
			}

			/**
			 * @brief Returns the number of mipmap levels.
			 * @return Number of mipmap levels.
			 */
			uint32_t GetMimMap() const
			{
				return m_mipmapLevel;
			}

		private:

			/**
			 * @brief Image width in pixels.
			 */
			uint32_t m_width = 0;

			/**
			 * @brief Image height in pixels.
			 */
			uint32_t m_height = 0;

			/**
			 * @brief Number of mipmap levels.
			 */
			uint32_t m_mipmapLevel = 1;

			/**
			 * @brief RAII Vulkan image handle.
			 */
			vkImage m_image = nullptr;

			/**
			 * @brief RAII Vulkan image view handle.
			 */
			vkImageView m_imageView = nullptr;

			/**
			 * @brief RAII Vulkan device memory for the image.
			 */
			vkDeviceMemory m_imageMemory = nullptr;
		};
	}
}