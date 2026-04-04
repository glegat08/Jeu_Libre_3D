#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Instance;

		/**
		 * @brief Stores Vulkan application and engine information for instance creation.
		 *
		 * This struct provides default values for application and engine names, versions,
		 * and Vulkan API version. It also wraps the Vulkan `vk::ApplicationInfo`.
		 */
		struct AppInfo
		{
			friend Instance;

			/**
			 * @brief Name of the application.
			 */
			const char* appName = "Basic_Api";

			/**
			 * @brief Name of the engine.
			 */
			const char* engineName = "None";

			/**
			 * @brief Version of the engine (using VK_MAKE_VERSION).
			 */
			std::uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);

			/**
			 * @brief Version of the application (using VK_MAKE_VERSION).
			 */
			std::uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0);

			/**
			 * @brief Vulkan API version used.
			 */
			std::uint32_t version = vk::ApiVersion13;

		private:
			/**
			 * @brief Initializes the Vulkan ApplicationInfo struct.
			 */
			void Create();

			/**
			 * @brief Vulkan application info handle.
			 */
			vk::ApplicationInfo m_info;
		};

		/**
		 * @brief Wrapper for a Vulkan instance with optional debug messenger.
		 *
		 * Encapsulates Vulkan instance creation, debug messenger setup, and RAII management.
		 */
		class Instance
		{
		public:

			/**
			 * @brief Type alias for a Vulkan debug callback function.
			 */
			using validCallBack = VkBool32(*)(vk::DebugUtilsMessageSeverityFlagBitsEXT, vk::DebugUtilsMessageTypeFlagsEXT, const vk::DebugUtilsMessengerCallbackDataEXT*, void*);

			/**
			 * @brief Type alias for Vulkan RAII instance.
			 */
			using vkInstance = vk::raii::Instance;

			/**
			 * @brief Type alias for Vulkan RAII debug utils messenger.
			 */
			using vkUtilMessenger = vk::raii::DebugUtilsMessengerEXT;

			/**
			 * @brief Default constructor.
			 */
			Instance() = default;

			/**
			 * @brief Constructs a Vulkan instance with application info and validation layers.
			 *
			 * @param appInfo Application info structure.
			 * @param validationLayers Vector of requested validation layers.
			 */
			Instance(AppInfo&&, std::vector<char const*> validationLayers);

			/**
			 * @brief Sets up a Vulkan debug messenger with a user-provided callback.
			 *
			 * @tparam cb User callback function.
			 */
			template<validCallBack cb>
			void setupDebugMessenger();

			/**
			 * @brief Returns the underlying Vulkan instance.
			 * @return Reference to the RAII Vulkan instance.
			 */
			vkInstance& Get();

			/**
			 * @brief Returns the underlying Vulkan instance (const).
			 * @return Const reference to the RAII Vulkan instance.
			 */
			const vkInstance& Get() const;

			/**
			 * @brief Returns the application info associated with this instance.
			 * @return Copy of AppInfo.
			 */
			AppInfo GetInfo() const;

		private:

			/**
			 * @brief Returns the list of required Vulkan extensions for instance creation.
			 * @return Vector of extension names.
			 */
			static std::vector<char const*> getRequiredExtensions();

			/**
			 * @brief Application info for this instance.
			 */
			AppInfo m_info;

			/**
			 * @brief Vulkan RAII instance handle.
			 */
			vkInstance m_instance = nullptr;

			/**
			 * @brief Vulkan RAII debug utils messenger handle.
			 */
			vkUtilMessenger m_debugMessenger = nullptr;
		};

		/**
		 * @brief Sets up the Vulkan debug messenger using a template callback.
		 *
		 * The messenger will only be created if validation layers are enabled.
		 *
		 * @tparam cb Callback function for debug messages.
		 */
		template <Instance::validCallBack cb>
		void Instance::setupDebugMessenger()
		{
			if (!enableValidationLayers)
				return;

			vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

			vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

			vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
				.messageSeverity = severityFlags,
				.messageType = messageTypeFlags,
				.pfnUserCallback = cb
			};

			m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
		}
	}
}
