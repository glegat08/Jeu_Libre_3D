#include <filesystem>
#include <iostream>
#include "Core/CameraComponent.h"

#include "imgui.h"
#include "VulkanCore.h"
#include "_GLFW.h"
#include "Backends/imgui_impl_glfw.h"
#include "Backends/imgui_impl_vulkan.h"
#include "Core/ManagerImple.h"
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "KGR_ImGui.h"

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	fileManager::SetGlobalFIlePath(projectRoot / "Ressources");
	STBManager::SetGlobalFIlePath(projectRoot / "Ressources");
	MeshLoader::SetGlobalFIlePath(projectRoot / "Ressources");

	KGR::_GLFW::Window::Init();
	KGR::_GLFW::Window::AddHint(GLFW_CLIENT_API, GLFW_NO_API);
	KGR::_GLFW::Window::AddHint(GLFW_RESIZABLE, GLFW_TRUE);
	KGR::_GLFW::Window window;
	window.CreateMyWindow({ 1400, 900 }, "GC goes Vulkan", nullptr, nullptr);

	KGR::_Vulkan::VulkanCore app(&window.GetWindow());
	KGR::_ImGui::ImGuiCore imguiCore;

	app.initVulkan();
	imguiCore.InitImGui(&app, &window);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	std::string modelPath = "Models\\viking_room.obj";
	std::string modelName = "viking_room.obj";
	MeshComponent meshComp;
	meshComp.mesh = &MeshLoader::Load(modelPath, &app);

	TransformComponent transform;

	CameraComponent cam = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);
	TransformComponent camTransform;

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1.5f);
	style.FontScaleDpi = 1.5f;

	glm::vec3 objPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 objScale    = { 2.0f, 2.0f, 2.0f };
	glm::vec3 objRotation = { glm::radians(-90.0f), 0.0f, 0.0f };

	static auto lastTime = std::chrono::high_resolution_clock::now();
	static float angle = 0.0f;
	const float rotationSpeed = 1.0f;

	do
	{
		KGR::_GLFW::Window::PollEvent();

		// Update
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		//angle += rotationSpeed * deltaTime;

		float radius = 5.0f;
		camTransform.SetPosition({ std::cos(angle) * radius, 0.0f, std::sin(angle) * radius });
		camTransform.LookAt({ 0.0f, 0.0f, 0.0f });
		cam.UpdateCamera(camTransform.GetFullTransform());

		// ImGui
		imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);
		{
			ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(360, 150), ImGuiCond_FirstUseEver);
			ImGui::Begin("Toto");

			ImGui::Text("File : %s", modelName.c_str());
			if (ImGui::Button("Load new.."))
			{
				std::string newPath = KGR::_ImGui::ImGuiCore::OpenFile();
				if (!newPath.empty())
				{
					app.GetDevice().Get().waitIdle();
					MeshLoader::Unload(modelPath);
					modelPath = newPath;
					modelName = std::filesystem::path(modelPath).filename().string();
					meshComp.mesh = &MeshLoader::Load(modelPath, &app);

					objPosition = { 0.0f, 0.0f, 0.0f };
					objScale    = { 1.0f, 1.0f, 1.0f };
					objRotation = { 0.0f, 0.0f, 0.0f };
				}
			}

			ImGui::Separator();

			ImGui::DragFloat3("Position",     &objPosition.x, 0.05f);
			ImGui::DragFloat3("Scale",        &objScale.x,    0.05f, 0.01f, 50.0f);
			ImGui::SliderAngle("Pitch (x)",   &objRotation.x, -180.0f, 180.0f);
			ImGui::SliderAngle("Yaw (y)",	   &objRotation.y, -180.0f, 180.0f);
			ImGui::SliderAngle("Roll (z)",    &objRotation.z, -180.0f, 180.0f);
			ImGui::End();
		}
		ImGui::Render();

		transform.SetPosition(objPosition);
		transform.SetScale(objScale);
		transform.SetRotation(objRotation);

		app.RegisterCam(cam, camTransform);
		app.RegisterRender(meshComp, transform);
		app.Render({ 0, 0, 0, 1 }, ImGui::GetDrawData());
	}
	while (!window.ShouldClose());

	app.GetDevice().Get().waitIdle();
	imguiCore.Destroy();
	window.DestroyMyWindow();
	MeshLoader::UnloadAll();
	KGR::_GLFW::Window::Destroy();
}