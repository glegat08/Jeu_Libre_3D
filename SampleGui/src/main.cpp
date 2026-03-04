#include <filesystem>
#include <iostream>
#include <cmath>
#include "Core/CameraComponent.h"

#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/LightComponent.h"
#include "KGR_ImGui.h"
#include "ObjectState.h"
#include "ObjectEditor.h"
#include "Core/Frenet.h"
#include "Core/Spline.h"
#include "DebugDraw3D.h"

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	FileManager::SetGlobalFIlePath(projectRoot / "Ressources");
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

	auto lComp3 = LightComponent<LightData::Type::Spot>::Create({ 0,1,0 }, { 1,1,1 }, 100.0f, 10.0f, glm::radians(45.0f), 10.0f);

	TransformComponent lTransform3;
	lTransform3.SetPosition({ -5,1,0 });
	lTransform3.LookAtDir({ 1,-1,0 });


	TextureComponent baseTexture;
	baseTexture.SetSize(1);
	baseTexture.AddTexture(0, &TextureLoader::Load("Textures\\BaseTexture.png", &app));

	std::vector<ObjectState> objects;
	int selectedObj = -1;

	auto  lastTime = std::chrono::high_resolution_clock::now();

	ObjectEditor objEditor(imguiCore, app);

	std::vector<glm::vec3> controlPoints = 
	{
		{  0.0f, 0.0f,  0.0f},
		{  3.0f, 0.0f,  4.0f},
		{  6.0f, 0.0f,  0.0f},
		{  3.0f, 0.0f, -4.0f},
		{  0.0f, 0.0f,  0.0f},
		{ -3.0f, 0.0f,  4.0f},
		{ -6.0f, 0.0f,  0.0f},
		{ -3.0f, 0.0f, -4.0f},
		{  0.0f, 0.0f,  0.0f},
		{  3.0f, 0.0f,  4.0f},
	};

	HermitCurve spline = HermitCurve::FromPoints(controlPoints, 0.0f);

	const int curveN = 400;
	std::vector<glm::vec3> curvePoints(curveN);
	float maxT = spline.MaxT();
	for (int i = 0; i < curveN; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(curveN) * maxT;
		curvePoints[i] = spline.Compute(std::min(t, maxT - 0.0001f));
	}
	auto curveTangents = KGR::RMF::EstimateForwardDirs(curvePoints);
	auto curveFrames = KGR::RMF::BuildFrames(curvePoints, curveTangents);
	const float axisLen = 0.3f;
	float curveTime = 0.0f;
	DebugDraw3D debugDraw;

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
					objEditor.DeleteSelected(objects, selectedObj);
				else if (!stillOpen)
					selectedObj = -1;
			}

			{
				debugDraw.BeginFrame(imguiCore.GetCam().GetView(), imguiCore.GetCam().GetProj(),
					io.DisplaySize.x, io.DisplaySize.y);

				// Curve
				for (int i = 0; i < curveN - 1; ++i)
					debugDraw.DrawLine(curvePoints[i], curvePoints[i + 1], IM_COL32(255, 255, 255, 255));
				debugDraw.DrawLine(curvePoints[curveN - 1], curvePoints[0], IM_COL32(255, 255, 255, 255));

				// Static frames
				int frameStep = curveN / 15;
				if (frameStep < 1) frameStep = 1;
				for (int i = 0; i < curveN; i += frameStep)
					debugDraw.DrawFrame(curvePoints[i], curveFrames[i], axisLen, 1.0f, 60);

				// Control points
				for (auto& cp : controlPoints)
					debugDraw.DrawPoint(cp, 5.0f, IM_COL32(255, 255, 0, 255));

				// Animated frame moving along the curve
				curveTime += deltaTime * 0.15f;
				curveTime = std::fmod(curveTime, 1.0f);

				float fIdx = curveTime * static_cast<float>(curveN);
				int   idx0 = static_cast<int>(fIdx) % curveN;
				int   idx1 = (idx0 + 1) % curveN;
				float alpha = fIdx - std::floor(fIdx);

				glm::vec3 pos = glm::mix(curvePoints[idx0], curvePoints[idx1], alpha);
				KGR::CurveFrame interpFrame;
				interpFrame.forward = glm::normalize(glm::mix(curveFrames[idx0].forward, curveFrames[idx1].forward, alpha));
				interpFrame.up      = glm::normalize(glm::mix(curveFrames[idx0].up,      curveFrames[idx1].up,      alpha));
				interpFrame.right   = glm::normalize(glm::cross(interpFrame.forward, interpFrame.up));

				debugDraw.DrawFrame(pos, interpFrame, axisLen * 2.5f, 3.5f);
				debugDraw.DrawPoint(pos, 6.0f, IM_COL32(255, 255, 0, 255));
			}

			KGR::_ImGui::ImGuiCore::SetWindow({ 20, 300 }, { 250, 120 }, "RMF Debug");
			ImGui::TextColored(ImVec4(0, 0, 1, 1),   "Blue  = Forward");
			ImGui::TextColored(ImVec4(0, 1, 0, 1),   "Green = Up");
			ImGui::TextColored(ImVec4(1, 0, 0, 1),   "Red   = Right");
			ImGui::End();
		}

		imguiCore.EndFrame();

		for (auto& obj : objects)
		{
			obj.ApplyTransform();
			if (obj.mesh.mesh)
				app.RegisterRender(*obj.mesh.mesh, obj.transform.GetFullTransform(), obj.texture.GetAllTextures());
		}

		app.RegisterCam(imguiCore.GetCamTransform().GetFullTransform(), imguiCore.GetCam().GetView(), imguiCore.GetCam().GetProj());

		{
			LightData ld1 = lComp.ToData();
			ld1.dir = lTransform.GetLocalAxe<RotData::Dir::Forward>();
			app.RegisterLight(ld1);
		}
		{
			LightData ld2 = lComp2.ToData();
			app.RegisterLight(ld2);
		}
		{
			LightData ld3 = lComp3.ToData();
			ld3.pos = lTransform3.GetPosition();
			ld3.dir = lTransform3.GetLocalAxe<RotData::Dir::Forward>();
			app.RegisterLight(ld3);
		}

		app.Render(&window.GetWindow(), { 0.53f, 0.81f, 0.92f, 1.0f }, imguiCore.GetDrawData());

	} while (!window.ShouldClose());

	app.GetDevice().Get().waitIdle();

	imguiCore.Destroy();
	window.DestroyMyWindow();
	MeshLoader::UnloadAll();
	TextureLoader::UnloadAll();
	KGR::_GLFW::Window::Destroy();
}