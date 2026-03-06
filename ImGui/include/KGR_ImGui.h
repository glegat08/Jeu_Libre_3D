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
		/// @brief Identifies which ImGui context is currently targeted.
		enum class ContextTarget
		{
			Engine, ///< The engine/editor context.
			Game    ///< The game/runtime context.
		};

		/// @brief Toolbar button types exposed by the engine UI.
		enum class ButtonType
		{
			Object,         ///< Add a generic object.
			Light,          ///< Add a light.
			Camera,         ///< Add a camera.
			Scene,          ///< Scene management.
			Load,           ///< Load a file.
			PlayAnimation,  ///< Start the current animation.
			StopAnimation,  ///< Stop the current animation.
			ResetObject     ///< Reset the selected object's transform.
		};

		/// @brief Wraps ImGui initialization, context management, camera control, and common UI helpers
		///        for the KGR engine.
		class ImGuiCore
		{
		public:

			/// @brief Initializes ImGui for the engine window and creates the engine context.
			/// @param vulkanCore Pointer to the Vulkan backend.
			/// @param engineWindow Pointer to the GLFW engine window.
			void InitImGui(KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* engineWindow);

			/// @brief Creates and configures an ImGui context for a given window.
			/// @param context      Reference to the context pointer to initialize.
			/// @param vulkanCore   Pointer to the Vulkan backend.
			/// @param window       Pointer to the target GLFW window.
			void InitContext(ImGuiContext*& context, KGR::_Vulkan::VulkanCore* vulkanCore, KGR::_GLFW::Window* window);

			/// @brief Switches the active ImGui context.
			/// @param target The context to activate (Engine or Game).
			void SetContext(ContextTarget target);

			/// @brief Begins a new ImGui frame on the specified context.
			/// @param target The context to use for this frame.
			void BeginFrame(ContextTarget target);

			// WORK IN PROGRESS
	/*		void CreateObject();
			void AddObject();
			void LoadObject();*/

			/// @brief Finalizes ImGui rendering for the current frame.
			void EndFrame();

			/// @brief Returns the ImGui draw data ready for submission to the renderer.
			/// @return Pointer to the current ImDrawData.
			ImDrawData* GetDrawData();

			/// @brief Returns the ImGuiIO for the active context.
			/// @return Reference to the current ImGuiIO.
			ImGuiIO& GetIO();

			/// @brief Binds a camera and its transform to the editor camera controller.
			/// @param cam       Pointer to the camera component.
			/// @param transform Pointer to the camera's transform component.
			/// @param speed     Movement speed in world units per second.
			void SetCamera(CameraComponent* cam, TransformComponent* transform, float speed = 5.0f);

			/// @brief Updates the editor camera based on mouse and keyboard input.
			/// @param deltaTime Time elapsed since the last frame in seconds.
			void UpdateCamera(float deltaTime);

			/// @brief Returns the bound camera component.
			/// @return Reference to the camera component.
			CameraComponent& GetCam();

			/// @brief Returns the bound camera transform component.
			/// @return Reference to the transform component.
			TransformComponent& GetCamTransform();

			/// @brief Shuts down ImGui and destroys all contexts.
			void Destroy();

			/// @brief Opens a file dialog, loads the chosen OBJ into a MeshComponent, and frees the previous mesh.
			/// @param meshComponent Target mesh component to populate.
			/// @param path          Current model path; updated to the new path on success.
			/// @param vkCore        Vulkan backend used to allocate mesh resources.
			/// @return true if a mesh was successfully loaded.
			bool LoadMesh(MeshComponent& meshComponent, std::string& path, _Vulkan::VulkanCore& vkCore);

			/// @brief Renders an ImGui button matching the given type and returns whether it was clicked.
			/// @param type The button to render.
			/// @return true if the button was clicked this frame.
			bool IsButton(ButtonType type);

			/// @brief Configures the next ImGui window position and size, then calls ImGui::Begin.
			/// @param position   Top-left corner of the window.
			/// @param size       Dimensions of the window.
			/// @param name       Window title.
			/// @param p_open     Optional pointer to a boolean controlling the window's open state.
			static void SetWindow(const ImVec2& position, const ImVec2& size, const char* name, bool* p_open = nullptr);

			/// @brief Opens a Windows file dialog filtered to OBJ files.
			/// @return The selected file path, or an empty string if cancelled.
			static std::string OpenFile();

		private:
			/// @brief Fills m_InitInfo with Vulkan handles required by ImGui_ImplVulkan.
			void InitInfo();

			template<typename ReturnType, typename WrapperType>
			ReturnType Get(WrapperType& type)
			{
				return static_cast<ReturnType>(*type.Get());
			}

			KGR::_Vulkan::VulkanCore* m_VulkanCore = nullptr;
			KGR::_GLFW::Window* m_Window = nullptr;
			ImGuiContext* m_EngineContext;
			ImGuiContext* m_GameContext;
			ImGui_ImplVulkan_InitInfo m_InitInfo = {};

			CameraComponent* m_Camera = nullptr;
			TransformComponent* m_CamTransform = nullptr;
			float m_CamSpeed = 5.0f;
			float m_Yaw = 0.0f;
			float m_Pitch = 0.0f;
			float m_MouseSensitivity = 0.15f;
			glm::dvec2 m_LastMousePos = { 0.0, 0.0 };
			bool m_IsRightClickActive = false;

			char m_ObjFilePath[512] = "";
			std::string m_LoadedObjName = "";
			bool m_LoadSuccess = false;
			bool m_LoadError = false;
		};
	}
}