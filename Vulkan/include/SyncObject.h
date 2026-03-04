#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class SwapChain; ///< Forward declaration of Vulkan swap chain
		class Device;    ///< Forward declaration of Vulkan logical device

		/**
		 * @brief Encapsulates Vulkan synchronization objects for frame rendering.
		 *
		 * Manages semaphores and fences for coordinating image acquisition,
		 * rendering completion, and frame progression in a swap chain.
		 */
		class SyncObject
		{
		public:

			/**
			 * @brief Alias for Vulkan RAII semaphore.
			 */
			using MyVkSemaphore = vk::raii::Semaphore;

			/**
			 * @brief Alias for Vulkan RAII fence.
			 */
			using MyvkFence = vk::raii::Fence;

			/**
			 * @brief Default constructor.
			 */
			SyncObject() = default;

			/**
			 * @brief Constructs synchronization objects for a given number of images.
			 *
			 * @param device Pointer to the Vulkan logical device.
			 * @param imageCount Number of swap chain images.
			 */
			SyncObject(Device* device, uint32_t imageCount);

			/**
			 * @brief Returns the semaphore used for signaling image acquisition completion.
			 * @return Reference to the current present semaphore.
			 */
			MyVkSemaphore& GetCurrentPresentSemaphore();

			/**
			 * @brief Returns the semaphore used for signaling image acquisition completion (const).
			 * @return Const reference to the current present semaphore.
			 */
			const MyVkSemaphore& GetCurrentPresentSemaphore() const;

			/**
			 * @brief Returns the semaphore used for signaling render completion.
			 * @return Reference to the current render semaphore.
			 */
			MyVkSemaphore& GetCurrentRenderSemaphore();

			/**
			 * @brief Returns the semaphore used for signaling render completion (const).
			 * @return Const reference to the current render semaphore.
			 */
			const MyVkSemaphore& GetCurrentRenderSemaphore() const;

			/**
			 * @brief Returns the fence associated with the current frame.
			 * @return Reference to the current fence.
			 */
			MyvkFence& GetCurrentFence();

			/**
			 * @brief Returns the fence associated with the current frame (const).
			 * @return Const reference to the current fence.
			 */
			const MyvkFence& GetCurrentFence() const;

			/**
			 * @brief Acquires the next image from the swap chain.
			 *
			 * @param swapChain Pointer to the swap chain.
			 * @param device Pointer to the Vulkan logical device.
			 * @return Index of the acquired swap chain image.
			 */
			uint32_t AcquireNextImage(SwapChain* swapChain, Device* device);

			/**
			 * @brief Advances the frame index for the next render.
			 */
			void IncrementFrame();

			/**
			 * @brief Returns the index of the current swap chain image.
			 * @return Current image index.
			 */
			const uint32_t& GetCurrentImage() const;

			/**
			 * @brief Returns the current frame index.
			 * @return Current frame index.
			 */
			const uint32_t& GetCurrentFrame() const;

		private:

			/**
			 * @brief Vector of semaphores used to signal that an image is ready for rendering.
			 */
			std::vector<MyVkSemaphore> m_presentCompleteSemaphores;

			/**
			 * @brief Vector of semaphores used to signal that rendering has finished.
			 */
			std::vector<MyVkSemaphore> m_renderFinishedSemaphores;

			/**
			 * @brief Vector of fences to synchronize CPU and GPU frame progress.
			 */
			std::vector<MyvkFence> m_inFlightFences;

			/**
			 * @brief Current frame index.
			 */
			uint32_t m_frameIndex;

			/**
			 * @brief Current swap chain image index.
			 */
			uint32_t m_imageIndex;

			/**
			 * @brief Total number of images in the swap chain.
			 */
			uint32_t m_imageCount;
		};
	}
}