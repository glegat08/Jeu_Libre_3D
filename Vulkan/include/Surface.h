#pragma once
#include "Global.h"

namespace KGR
{
	namespace _Vulkan
	{
		class Instance; ///< Forward declaration of Vulkan instance wrapper

		/**
		 * @brief Wrapper for a Vulkan surface using RAII.
		 *
		 * Encapsulates creation and management of a Vulkan surface for a GLFW window.
		 */
		class Surface
		{
			/**
			 * @brief Alias for Vulkan RAII surface type.
			 */
			using vkSurface = vk::raii::SurfaceKHR;

		public:

			/**
			 * @brief Default constructor.
			 */
			Surface() = default;

			/**
			 * @brief Constructs a Vulkan surface for a GLFW window.
			 *
			 * @param instance Pointer to the Vulkan instance.
			 * @param window Pointer to the GLFW window.
			 */
			Surface(Instance* instance, GLFWwindow* window);

			/**
			 * @brief Returns the Vulkan surface.
			 * @return Reference to the RAII Vulkan surface.
			 */
			vkSurface& Get();

			/**
			 * @brief Returns the Vulkan surface (const).
			 * @return Const reference to the RAII Vulkan surface.
			 */
			const vkSurface& Get() const;

		private:

			/**
			 * @brief RAII Vulkan surface handle.
			 */
			vkSurface m_surface = nullptr;
		};
	}
}