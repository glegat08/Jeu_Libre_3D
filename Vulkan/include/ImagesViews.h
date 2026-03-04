#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class SwapChain; ///< Forward declaration of Vulkan swap chain
		class Device;    ///< Forward declaration of Vulkan logical device

		/**
		 * @brief Container for multiple Vulkan image views.
		 *
		 * Provides creation and management of image views for swap chains or images,
		 * supporting different view types (1D, 2D, 3D, cube, and array variants).
		 */
		class ImagesViews
		{
		public:

			/**
			 * @brief Enum representing supported image view types.
			 *
			 * Maps to Vulkan VkImageViewType:
			 * - vt1D       = VK_IMAGE_VIEW_TYPE_1D
			 * - vt2D       = VK_IMAGE_VIEW_TYPE_2D
			 * - vt3D       = VK_IMAGE_VIEW_TYPE_3D
			 * - vtCube     = VK_IMAGE_VIEW_TYPE_CUBE
			 * - vt1DArray  = VK_IMAGE_VIEW_TYPE_1D_ARRAY
			 * - vt2DArray  = VK_IMAGE_VIEW_TYPE_2D_ARRAY
			 * - vtCubeArray= VK_IMAGE_VIEW_TYPE_CUBE_ARRAY
			 */
			enum class ViewType
			{
				vt1D,
				vt2D,
				vt3D,
				vtCube,
				vt1DArray,
				vt2DArray,
				vtCubeArray
			};

			/**
			 * @brief Alias for Vulkan RAII image view type.
			 */
			using vkImageView = vk::raii::ImageView;

			/**
			 * @brief Alias for a vector of Vulkan image views.
			 */
			using vkImagesViews = std::vector<vkImageView>;

			/**
			 * @brief Default constructor.
			 */
			ImagesViews() = default;

			/**
			 * @brief Constructs image views for a swap chain.
			 *
			 * @param swapChain Pointer to the Vulkan swap chain.
			 * @param device Pointer to the Vulkan logical device.
			 * @param viewType Desired image view type.
			 */
			ImagesViews(SwapChain* swapChain, Device* device, const ViewType& viewType);

			/**
			 * @brief Returns the vector of Vulkan image views.
			 * @return Reference to the vector of image views.
			 */
			vkImagesViews& Get();

			/**
			 * @brief Returns the vector of Vulkan image views (const).
			 * @return Const reference to the vector of image views.
			 */
			const vkImagesViews& Get() const;

		private:

			/**
			 * @brief Converts a KGR ViewType enum to the corresponding Vulkan VkImageViewType.
			 *
			 * @param viewType KGR view type.
			 * @return Vulkan VkImageViewType equivalent.
			 */
			static vk::ImageViewType KGRToVulkan(const ViewType& viewType);

			/**
			 * @brief Container holding the Vulkan RAII image views.
			 */
			vkImagesViews m_views;
		};
	}
}