#pragma once

#include "VulkanCore.h"
#include "_GLFW.h"
#include "Backends/imgui_impl_vulkan.h"
#include "Core/ManagerImple.h"

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

		struct ImGuiInitInfo
		{
			uint32_t                        ApiVersion;
			VkInstance                      Instance;
			VkPhysicalDevice                PhysicalDevice;
			VkDevice                        Device;
			uint32_t                        QueueFamily;
			VkQueue                         Queue;
			VkDescriptorPool                DescriptorPool;
			uint32_t                        MinImageCount;
			uint32_t                        ImageCount;
			ImGui_ImplVulkan_PipelineInfo   PipelineInfoMain;
			bool                            UseDynamicRendering;
			VkFormat						ColorFormat;
			VkFormat						DepthFormat;
		};

		class ImGuiCore
		{
		public:
			ImGuiCore();
			~ImGuiCore();

			void InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow);

			void BeginFrame(ContextTarget target);
			void EndFrame(ContextTarget target, VkCommandBuffer commandBuffer);
			void Render(ContextTarget target);
			
			void SetContext(ContextTarget target);

			void Destroy();

		private:
			template<typename ReturnType, typename WrapperType>
			ReturnType Get(WrapperType& type)
			{
				return static_cast<ReturnType>(*type.Get());
			}

			void InitInfo(ImGuiInitInfo& initInfo);
			void InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* window);

			KGR::_Vulkan::VulkanCore* m_VulkanCore = nullptr;
			ImGuiContext* m_EngineContext = nullptr;
			ImGuiContext* m_GameContext = nullptr;

			char		m_ObjFilePath[512] = "";
			std::string m_LoadedObjName    = "";
			bool		m_LoadSuccess	   = false;
			bool		m_LoadError		   = false;
		};
	}
}
