#pragma once

#include "VulkanCore.h"
#include "_GLFW.h"
#include "Backends/imgui_impl_vulkan.h"
#include "Core/ManagerImple.h"
#include <ImGuizmo.h>

struct ImGuiContext;

namespace KGR
{
	namespace _ImGui
	{
		enum class ContextTarget
		{
			Engine,
			Game
		};

		class ImGuiCore
		{
		public:

			void InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow);
			void InitInfo();
			void InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* window);

			void BeginFrame(ContextTarget target);
			void EndFrame(ContextTarget target, VkCommandBuffer commandBuffer);
			void Render(ContextTarget target);
			
			void SetContext(ContextTarget target);

			void Destroy();

			static std::string OpenFile();

		private:
			template<typename ReturnType, typename WrapperType>
			ReturnType Get(WrapperType& type)
			{
				return static_cast<ReturnType>(*type.Get());
			}

			KGR::_Vulkan::VulkanCore* m_VulkanCore = nullptr;
			ImGuiContext* m_EngineContext		   = nullptr;
			ImGuiContext* m_GameContext			   = nullptr;
			ImGui_ImplVulkan_InitInfo m_InitInfo   = {};

			char		m_ObjFilePath[512] = "";
			std::string m_LoadedObjName    = "";
			bool		m_LoadSuccess	   = false;
			bool		m_LoadError		   = false;
		};
	}
}
