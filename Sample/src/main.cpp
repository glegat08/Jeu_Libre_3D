#include <filesystem>
#include <iostream>
#include "Core/CameraComponent.h"

#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/LightComponent.h"
#include "KGR_ImGui.h"
#include "ObjectState.h"
#include "ObjectEditor.h"

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	fileManager::SetGlobalFIlePath(projectRoot / "Ressources");
	STBManager::SetGlobalFIlePath(projectRoot / "Ressources");
	MeshLoader::SetGlobalFIlePath(projectRoot / "Ressources");
	TextureLoader::SetGlobalFIlePath(projectRoot / "Ressources");

	KGR::_GLFW::Window::Init();
	KGR::_GLFW::Window::AddHint(GLFW_CLIENT_API, GLFW_NO_API);
	KGR::_GLFW::Window::AddHint(GLFW_RESIZABLE, GLFW_TRUE);
	KGR::_GLFW::Window window;
	window.CreateMyWindow({ 1400, 900 }, "GC goes Vulkan", nullptr, nullptr);

	KGR::_Vulkan::VulkanCore app{};
	KGR::_ImGui::ImGuiCore imguiCore;

	app.initVulkan(&window.GetWindow());
	imguiCore.InitImGui(&app, &window);

	ImGuiIO& io = imguiCore.GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	CameraComponent cam = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);
	
	TransformComponent camTransform;
	camTransform.SetPosition({ 0.0f, 5.0f, 5.0f });
	camTransform.LookAt({ 0.0f, 0.0f, 0.0f });
	cam.UpdateCamera(camTransform.GetFullTransform());
	imguiCore.SetCamera(&cam, &camTransform);

	auto lComp = LightComponent<LightData::Type::Directional>::Create({ 1,1,1 }, { 1,1,1 }, 10.0f);

	TransformComponent lTransform;


	lTransform.LookAtDir({ 1,-1,1 });
	auto loclAxes = lTransform.GetLocalAxe<RotData::Dir::Forward>();
	std::cout << loclAxes.z;


	auto lComp2 = LightComponent<LightData::Type::Point>::Create({ 0,0,1 }, { 1,1,1 }, 10.0f, 10.0f);

	TransformComponent lTransform2;
	lTransform2.SetPosition({ 5,1,0 });

	auto lComp3 = LightComponent<LightData::Type::Spot>::Create({ 0,1,0 }, { 1,1,1 }, 100.0f, 10.0f, glm::radians(45.0f), 10.0f);

	TransformComponent lTransform3;
	lTransform3.SetPosition({ -5,1,0 });
	lTransform3.LookAtDir({ 1,-1,0 });


	TextureComponent baseTexture;
	baseTexture.texture = &TextureLoader::Load("Textures\\BaseTexture.png", &app);

	std::vector<ObjectState> objects;
	int selectedObj = -1;

	auto  lastTime = std::chrono::high_resolution_clock::now();

	ObjectEditor objEditor(imguiCore, app);

	do
	{
		KGR::_GLFW::Window::PollEvent();

		// Update
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		imguiCore.UpdateCamera(deltaTime);

		for (auto& obj : objects)
			if (obj.isAnimating)
				obj.rotation.y += glm::radians(90.0f) * deltaTime;

		// ImGui
		imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);
		{
			KGR::_ImGui::ImGuiCore::SetWindow({ 400, 20 }, { 500, 200 }, "KGR Engine");
			ImGui::Text("Welcome to the KGR Engine !\n\nUse right click and ZQSD to move the camera.");

			if (imguiCore.IsButton(KGR::_ImGui::ButtonType::Object))
			{
				ObjectState& obj = objects.emplace_back();
				obj.name		 = "Object " + std::to_string(objects.size() - 1);
				obj.texture		 = baseTexture;
				selectedObj		 = static_cast<int>(objects.size() - 1);
			}

			ImGui::Separator();

			for (int i = 0; i < static_cast<int>(objects.size()); i++)
				if (ImGui::Selectable(objects[i].name.c_str(), selectedObj == i))
					selectedObj = i;

			ImGui::End();

			if (selectedObj >= 0 && selectedObj < static_cast<int>(objects.size()))
			{
				objEditor.SetTarget(&objects[selectedObj]);

				bool stillOpen = objEditor.Render();

				if (objEditor.DeleteObject())
				{
					app.GetDevice().Get().waitIdle();

					if (!objects[selectedObj].modelPath.empty())
						MeshLoader::Unload(objects[selectedObj].modelPath);

					objects.erase(objects.begin() + selectedObj);
					selectedObj = -1;
				}
				else if (!stillOpen)
					selectedObj = -1;
			}
		}

		imguiCore.EndFrame();

		for (auto& obj : objects)
		{
			obj.ApplyTransform();
			app.RegisterRender(obj.mesh, obj.transform, obj.texture);
		}

		app.RegisterCam(imguiCore.GetCam(), imguiCore.GetCamTransform());

		app.RegisterLight(lComp, lTransform);
		app.RegisterLight(lComp2, lTransform2);
		app.RegisterLight(lComp3, lTransform3);

		app.Render(&window.GetWindow(), { 0.53f, 0.81f, 0.92f, 1.0f }, imguiCore.GetDrawData());

	} while (!window.ShouldClose());

	app.GetDevice().Get().waitIdle();

	imguiCore.Destroy();
	window.DestroyMyWindow();
	MeshLoader::UnloadAll();
	TextureLoader::UnloadAll();
	KGR::_GLFW::Window::Destroy();
}