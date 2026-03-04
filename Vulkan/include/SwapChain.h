#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Surface;         ///< Forward declaration of Vulkan surface
		class Device;          ///< Forward declaration of Vulkan logical device
		class PhysicalDevice;  ///< Forward declaration of Vulkan physical device

		/**
		 * @brief Wrapper for a Vulkan swap chain using RAII.
		 *
		 * Handles creation, access, and management of swap chain images, formats,
		 * extents, and presentation modes. Provides utilities for choosing suitable
		 * swap chain parameters.
		 */
		class SwapChain
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII swap chain type.
			 */
			using vkSwapChain = vk::raii::SwapchainKHR;

			/**
			 * @brief Alias for Vulkan surface format type.
			 */
			using vkFormat = vk::SurfaceFormatKHR;

			/**
			 * @brief Alias for Vulkan 2D extent type.
			 */
			using vkExtend = vk::Extent2D;

			/**
			 * @brief Alias for Vulkan image type.
			 */
			using vkImage = vk::Image;

			/**
			 * @brief Enum representing desired swap chain presentation modes.
			 */
			enum class PresMode
			{
				Immediate,
				Mailbox,
				Fifo,
				FifoRelaxed,
				SharedDemandRefresh,
				SharedContinuousRefresh,
				FifoLatestReady,
				FifoLatestReadyEXT,
			};

			/**
			 * @brief Default constructor.
			 */
			SwapChain() = default;

			/**
			 * @brief Constructs a Vulkan swap chain with desired parameters.
			 *
			 * @param pDevice Pointer to the Vulkan physical device.
			 * @param device Pointer to the Vulkan logical device.
			 * @param surface Pointer to the Vulkan surface.
			 * @param window Pointer to the GLFW window.
			 * @param wanted Desired presentation mode.
			 * @param imageCount Number of swap chain images (default 3).
			 * @param old Pointer to an old swap chain to replace (optional).
			 */
			SwapChain(PhysicalDevice* pDevice,
				Device* device,
				Surface* surface,
				GLFWwindow* window,
				PresMode wanted,
				ui32t imageCount = 3,
				SwapChain* old = nullptr);

			/**
			 * @brief Returns the Vulkan swap chain handle.
			 * @return Reference to the RAII Vulkan swap chain.
			 */
			vkSwapChain& Get();

			/**
			 * @brief Returns the Vulkan swap chain handle (const).
			 * @return Const reference to the RAII Vulkan swap chain.
			 */
			const vkSwapChain& Get() const;

			/**
			 * @brief Returns the swap chain extent (width and height).
			 * @return Vulkan extent 2D.
			 */
			vkExtend GetExtend() const;

			/**
			 * @brief Returns the swap chain surface format.
			 * @return Reference to the Vulkan surface format.
			 */
			vkFormat& GetFormat();

			/**
			 * @brief Returns the swap chain surface format (const).
			 * @return Const reference to the Vulkan surface format.
			 */
			const vkFormat& GetFormat() const;

			/**
			 * @brief Returns the swap chain images.
			 * @return Reference to vector of Vulkan images.
			 */
			std::vector<vkImage>& GetImages();

			/**
			 * @brief Returns the swap chain images (const).
			 * @return Const reference to vector of Vulkan images.
			 */
			const std::vector<vkImage>& GetImages() const;

			/**
			 * @brief Returns the number of images in the swap chain.
			 * @return Number of swap chain images.
			 */
			uint32_t GetImagesCount() const;

			/**
			 * @brief Chooses the swap chain extent based on surface capabilities and window size.
			 *
			 * @param capabilities Vulkan surface capabilities.
			 * @param window Pointer to the GLFW window.
			 * @return Chosen Vulkan extent.
			 */
			static vkExtend chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

			/**
			 * @brief Chooses the minimum number of swap chain images.
			 *
			 * @param surfaceCapabilities Vulkan surface capabilities.
			 * @param imageTargetCount Desired image count.
			 * @return Selected minimum image count.
			 */
			static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities, uint32_t imageTargetCount);

			/**
			 * @brief Chooses a suitable swap chain surface format from available formats.
			 *
			 * @param availableFormats Vector of available surface formats.
			 * @return Selected Vulkan surface format.
			 */
			static vkFormat chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

			/**
			 * @brief Chooses a suitable Vulkan presentation mode.
			 *
			 * @param availablePresentModes Vector of available presentation modes.
			 * @param wanted Desired KGR presentation mode.
			 * @return Selected Vulkan presentation mode.
			 */
			static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, PresMode wanted);

		private:

			/**
			 * @brief Checks whether a Vulkan presentation mode matches the desired KGR mode.
			 *
			 * @param get Vulkan presentation mode.
			 * @param wanted Desired KGR presentation mode.
			 * @return True if matches, false otherwise.
			 */
			static bool IsWantedPresentModel(vk::PresentModeKHR get, PresMode wanted);

			/**
			 * @brief Converts a KGR presentation mode to a Vulkan presentation mode.
			 *
			 * @param wanted Desired KGR presentation mode.
			 * @return Corresponding Vulkan presentation mode.
			 */
			static vk::PresentModeKHR KGRToVulkan(PresMode wanted);

			/**
			 * @brief RAII Vulkan swap chain handle.
			 */
			vkSwapChain m_swapChain = nullptr;

			/**
			 * @brief Swap chain surface format.
			 */
			vkFormat m_swapChainSurfaceFormat;

			/**
			 * @brief Swap chain extent (width and height).
			 */
			vkExtend m_swapChainExtent;

			/**
			 * @brief Vector of images in the swap chain.
			 */
			std::vector<vkImage> m_swapChainImages;

			/**
			 * @brief Number of images in the swap chain.
			 */
			uint32_t m_imageCount;
		};
	}
}