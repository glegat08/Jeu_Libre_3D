#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Instance; ///< Forward declaration of Vulkan instance wrapper

		/**
		 * @brief Wrapper for a Vulkan physical device using RAII.
		 *
		 * Handles selection of a physical device based on a desired device type
		 * and provides utility functions for querying supported formats.
		 */
		class PhysicalDevice
		{
		public:

			/**
			 * @brief Enum representing preferred physical device types.
			 */
			enum class DeviceType
			{
				Best,        ///< Select the best available device
				Integrated,  ///< Integrated GPU
				Discrete,    ///< Discrete GPU
				Virtual,     ///< Virtual GPU
				Cpu,         ///< CPU device
				Other        ///< Other device type
			};

			/**
			 * @brief Alias for Vulkan format type.
			 */
			using vkFormat = vk::Format;

			/**
			 * @brief Alias for Vulkan RAII physical device type.
			 */
			using vkPhysicDevice = vk::raii::PhysicalDevice;

			/**
			 * @brief Default constructor.
			 */
			PhysicalDevice() = default;

			/**
			 * @brief Selects and constructs a physical device from a Vulkan instance.
			 *
			 * @param instance Pointer to the Vulkan instance.
			 * @param wanted Desired device type.
			 */
			PhysicalDevice(Instance* instance, DeviceType wanted);

			/**
			 * @brief Returns the underlying Vulkan physical device.
			 * @return Reference to the Vulkan RAII physical device.
			 */
			vkPhysicDevice& Get();

			/**
			 * @brief Returns the underlying Vulkan physical device (const).
			 * @return Const reference to the Vulkan RAII physical device.
			 */
			const vkPhysicDevice& Get() const;

			/**
			 * @brief Finds a supported image format from a list of candidates.
			 *
			 * @param candidates List of candidate Vulkan formats.
			 * @param tiling Image tiling mode.
			 * @param features Required format feature flags.
			 * @return Supported Vulkan format.
			 */
			vkFormat findSupportedFormat(const std::vector<vk::Format>& candidates,
				vk::ImageTiling tiling, vk::FormatFeatureFlags features);

		private:

			/**
			 * @brief Checks whether a Vulkan physical device type matches the desired type.
			 *
			 * @param get Vulkan physical device type.
			 * @param wanted Desired device type.
			 * @return True if matching, false otherwise.
			 */
			static bool IsMatchingDeviceType(vk::PhysicalDeviceType get, DeviceType wanted);

			/**
			 * @brief RAII Vulkan physical device handle.
			 */
			vkPhysicDevice m_device = nullptr;
		};
	}
}