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

	MeshComponent meshComp;
	meshComp.mesh = &MeshLoader::Load("Models\\viking_room.obj", &app);
	MeshComponent meshComp2;
	meshComp2.mesh = &MeshLoader::Load("Models\\briet_claire_decorsfantasy_grpB.obj", &app);
	TransformComponent transform;
	transform.SetPosition(glm::vec3(0, 0, 0));
	transform.SetScale({2, 2, 2});
	transform.RotateQuat<RotData::Orientation::Pitch>(glm::radians(-90.0f));
	TransformComponent transform2;
	transform2.RotateQuat<RotData::Orientation::Pitch>(glm::radians(-90.0f));

	CameraComponent cam = CameraComponent :: Create(45.0f,static_cast<float>(window.GetSize().x),static_cast<float>(window.GetSize().y),0.01f,1000.0f,CameraComponent::Type::Perspective);
	TransformComponent camTransform;




	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(1.f);
	style.FontScaleDpi = 3.0f;

	do
	{
		// event
		KGR::_GLFW::Window::PollEvent();
		//Update
		static auto lastTime = std::chrono::high_resolution_clock::now();
		static float angle = 0.0f;
		const float rotationSpeed = 1.0f;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		angle += deltaTime * rotationSpeed;


		float radius = 5.0f;
		float camX = std::cos(angle) * radius;
		float camY = 0.0f;
		float camZ = std::sin(angle) * radius;

		camTransform.SetPosition({ camX, camY, camZ });
		camTransform.LookAt({ 0.0f, 0.0f, 0.0f });
		cam.UpdateCamera(camTransform.GetFullTransform());

		// Render
		
		app.RegisterCam(cam, camTransform);
		app.RegisterRender(meshComp, transform);
		app.Render();
	
	}
	while (!window.ShouldClose());

	 window.DestroyMyWindow();
	 MeshLoader::UnloadAll();
     KGR::_GLFW::Window::Destroy();

		imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);

		{
			static int counter;

			ImGui::Begin("LE H");
			if (ImGui::Button("Click on me daddy"))
				counter++;

			ImGui::Text("Daddy Ludwig <3 = %d", counter);

			ImGui::End();
		}

		 ImGui::Render();
		 app.drawFrame(ImGui::GetDrawData());
	} 
	while (!window.ShouldClose());

	app.GetDevice().Get().waitIdle();

	imguiCore.Destroy();
	window.DestroyMyWindow();
    KGR::_GLFW::Window::Destroy();

}
