#pragma once

#include "VulkanCore.h"
#include "_GLFW.h"
#include "imgui.h"
#include "Backends/imgui_impl_vulkan.h"
#include "Core/ManagerImple.h"
#include "Core/CameraComponent.h"
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

		enum class ButtonType
		{
			Object,
			Light,
			Camera,
			Scene,
			Load,
			PlayAnimation,
			StopAnimation,
			ResetObject
		};

		class ImGuiCore
		{
		public:

			void InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow);
			void InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* window);
			void SetContext(ContextTarget target);
			void BeginFrame(ContextTarget target);

			void CreateObject();
			void AddObject();
			void LoadObject();

			void EndFrame();
			ImDrawData* GetDrawData();
			ImGuiIO& GetIO();

			void SetCamera(CameraComponent* cam, TransformComponent* transform, float speed = 5.0f);
			void UpdateCamera(float deltaTime);
			CameraComponent& GetCam();
			TransformComponent& GetCamTransform();

			void Destroy();

			bool LoadMesh(MeshComponent& meshComponent, std::string& path, _Vulkan::VulkanCore& vkCore);
			bool IsButton(ButtonType type);

			static void SetWindow(const ImVec2& position, const ImVec2& size, const char* name, bool* p_open = nullptr);
			static std::string OpenFile();

		private:
			void InitInfo();

			template<typename ReturnType, typename WrapperType>
			ReturnType Get(WrapperType& type)
			{
				return static_cast<ReturnType>(*type.Get());
			}

			KGR::_Vulkan::VulkanCore* m_VulkanCore = nullptr;
			KGR::_GLFW::Window*       m_Window     = nullptr;
			ImGuiContext* m_EngineContext;
			ImGuiContext* m_GameContext;
			ImGui_ImplVulkan_InitInfo m_InitInfo = {};

			CameraComponent*    m_Camera             = nullptr;
			TransformComponent* m_CamTransform       = nullptr;
			float               m_CamSpeed           = 5.0f;
			float               m_Yaw                = 0.0f;
			float               m_Pitch              = 0.0f;
			float               m_MouseSensitivity   = 0.15f;
			glm::dvec2          m_LastMousePos       = { 0.0, 0.0 };
			bool                m_IsRightClickActive = false;

			char		m_ObjFilePath[512] = "";
			std::string m_LoadedObjName    = "";
			bool		m_LoadSuccess	   = false;
			bool		m_LoadError		   = false;
		};
	}
}
