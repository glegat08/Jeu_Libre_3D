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
#include <glm/gtc/constants.hpp>   // glm::two_pi
#include "Core/Mesh.h"             // Vertex, Mesh, SubMeshes


//----------------------------------------------------------------
//----------------------------------------------------------------

// WORK IN PROGRESS
// THERE ARE SOME FRENCH COMMENTS FOR ALGORITHMS FOUND BY AI.
// I LET THE COMMENTS HERE TO UNDERSTAND HOW IT WORKS BEFORE REWORKING IT.

//----------------------------------------------------------------
//----------------------------------------------------------------


// ---------------------------------------------------------------------------
// Génère la grille aplatie de vertices du tube.
// Index d'un sommet dans le tableau : i * tubeSegments + j
//   i = position le long de la courbe  (0 .. curveN-1)
//   j = position autour de l'anneau    (0 .. tubeSegments-1)
//
// La normale est la direction radiale pure (cos*right + sin*up), déjà unitaire.
// Les UVs mappent u autour de l'anneau et v le long de la courbe.
// ---------------------------------------------------------------------------
static std::vector<Vertex> BuildTubeVertices(
	const std::vector<glm::vec3>& curvePoints,
	const std::vector<KGR::CurveFrame>& curveFrames,
	float radius,
	int   tubeSegments)
{
	const int curveN = static_cast<int>(curvePoints.size());
	std::vector<Vertex> vertices;
	vertices.reserve(curveN * tubeSegments);

	for (int i = 0; i < curveN; ++i)
	{
		for (int j = 0; j < tubeSegments; ++j)
		{
			float angle = (static_cast<float>(j) / static_cast<float>(tubeSegments))
				* glm::two_pi<float>();

			// Direction radiale dans le plan local (right, up) du frame RMF
			glm::vec3 radialDir = curveFrames[i].right * std::cos(angle)
				+ curveFrames[i].up * std::sin(angle);

			Vertex v;
			v.pos = curvePoints[i] + radialDir * radius;
			v.normal = radialDir;   // déjà normalisé par construction
			v.uv = { static_cast<float>(j) / static_cast<float>(tubeSegments),
						 static_cast<float>(i) / static_cast<float>(curveN - 1) };
			v.color = glm::vec4(1.0f);

			vertices.push_back(v);
		}
	}
	return vertices;
}

// ---------------------------------------------------------------------------
// Génère les indices (triangles) pour une grille torique curveN x tubeSegments.
// Chaque quad (i,j) → (i+1,j) → (i+1,j+1) → (i,j+1) est découpé en 2 triangles.
// Les modulos referment la surface dans les deux directions (tore fermé).
// ---------------------------------------------------------------------------
static std::vector<uint32_t> BuildTubeIndices(int curveN, int tubeSegments)
{
	std::vector<uint32_t> indices;
	indices.reserve(curveN * tubeSegments * 6);

	auto idx = [&](int i, int j) -> uint32_t
		{
			return static_cast<uint32_t>((i % curveN) * tubeSegments + (j % tubeSegments));
		};

	for (int i = 0; i < curveN; ++i)
	{
		for (int j = 0; j < tubeSegments; ++j)
		{
			uint32_t v00 = idx(i, j);
			uint32_t v10 = idx(i + 1, j);
			uint32_t v01 = idx(i, j + 1);
			uint32_t v11 = idx(i + 1, j + 1);

			// Triangle 1
			indices.push_back(v00);
			indices.push_back(v10);
			indices.push_back(v11);

			// Triangle 2
			indices.push_back(v00);
			indices.push_back(v11);
			indices.push_back(v01);
		}
	}
	return indices;
}

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

	auto lastTime = std::chrono::high_resolution_clock::now();

	ObjectEditor objEditor(imguiCore, app);

	std::vector<glm::vec3> controlPoints =
	{
		{ -3.0f, 0.0f,  4.0f},
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
		{  6.0f, 0.0f,  0.0f},
		{  3.0f, 0.0f, -4.0f},
		{  0.0f, 0.0f,  0.0f},
		{ -3.0f, 0.0f, -4.0f},
		{ -6.0f, 0.0f,  0.0f},
		{ -3.0f, 0.0f,  4.0f},
		{  0.0f, 0.0f,  0.0f},
		{  3.0f, 0.0f,  4.0f},
	};

	HermitCurve spline = HermitCurve::FromPoints(controlPoints, 0.0f);

	const int curveN = 100;
	std::vector<glm::vec3> curvePoints;
	float maxT = spline.MaxT();

	for (int i = 0; i < curveN; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(curveN - 1) * maxT;
		curvePoints.push_back(spline.Compute(t));
	}

	auto curveTangents = KGR::RMF::EstimateForwardDirs(curvePoints);
	auto curveFrames = KGR::RMF::BuildFrames(curvePoints, curveTangents);

	// ---------------------------------------------------------------------------
	// Génération du mesh du tube — fait une seule fois avant la boucle.
	// On produit les vertices (pos, normal, uv) et les indices, puis on les
	// envoie directement à VulkanCore via le constructeur de SubMeshes qui
	// alloue le vertex buffer et l'index buffer sur le GPU.
	// ---------------------------------------------------------------------------
	const int tubeSegments = 12;
	const float tubeRadius = 0.18f;

	auto tubeVertices = BuildTubeVertices(curvePoints, curveFrames, tubeRadius, tubeSegments);
	auto tubeIndices = BuildTubeIndices(curveN, tubeSegments);

	Mesh tubeMesh;
	tubeMesh.AddSubMesh(std::make_unique<SubMeshes>(tubeVertices, tubeIndices, "tube", &app));

	const float axisLen = 0.3f;
	float curveTime = 0.0f;
	DebugDraw3D debugDraw;

	do
	{
		KGR::_GLFW::Window::PollEvent();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		imguiCore.UpdateCamera(deltaTime);

		for (auto& obj : objects)
			if (obj.isAnimating)
				obj.rotation.y += glm::radians(90.0f) * deltaTime;

		imguiCore.BeginFrame(KGR::_ImGui::ContextTarget::Engine);
		{
			KGR::_ImGui::ImGuiCore::SetWindow({ 400, 20 }, { 500, 200 }, "KGR Engine");
			ImGui::Text("Welcome to the KGR Engine !\n\nUse right click and ZQSD to move the camera.");

			if (imguiCore.IsButton(KGR::_ImGui::ButtonType::Object))
			{
				ObjectState& obj = objects.emplace_back();
				obj.name = "Object " + std::to_string(objects.size() - 1);
				obj.texture = baseTexture;
				selectedObj = static_cast<int>(objects.size() - 1);
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

				for (int i = 0; i < curveN - 1; ++i)
					debugDraw.DrawLine(curvePoints[i], curvePoints[i + 1], IM_COL32(255, 255, 255, 255));

				debugDraw.DrawLine(curvePoints[curveN - 1], curvePoints[0], IM_COL32(255, 255, 255, 255));

				for (auto& cp : controlPoints)
					debugDraw.DrawPoint(cp, 5.0f, IM_COL32(255, 255, 0, 255));

				curveTime += deltaTime * 0.09f;

				float sampleIndexF = curveTime * static_cast<float>(curveN);
				int sampleIdx = static_cast<int>(sampleIndexF) % curveN;
				int nextSampleIdx = (sampleIdx + 1) % curveN;
				float lerpFactor = sampleIndexF - std::floor(sampleIndexF);

				glm::vec3 pos = glm::mix(curvePoints[sampleIdx], curvePoints[nextSampleIdx], lerpFactor);

				KGR::CurveFrame interpFrame;
				interpFrame.forward = glm::normalize(glm::mix(curveFrames[sampleIdx].forward, curveFrames[nextSampleIdx].forward, lerpFactor));
				interpFrame.up = glm::normalize(glm::mix(curveFrames[sampleIdx].up, curveFrames[nextSampleIdx].up, lerpFactor));
				interpFrame.right = glm::normalize(glm::cross(interpFrame.forward, interpFrame.up));

				debugDraw.DrawFrame(pos, interpFrame, axisLen * 2.5f, 3.5f);
				debugDraw.DrawPoint(pos, 6.0f, IM_COL32(255, 255, 0, 255));
			}

			KGR::_ImGui::ImGuiCore::SetWindow({ 20, 300 }, { 250, 120 }, "RMF Debug");
			ImGui::TextColored(ImVec4(0, 0, 1, 1), "Blue  = Forward");
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Green = Up");
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Red   = Right");
			ImGui::End();
		}

		imguiCore.EndFrame();

		// Tube mesh — identité comme transform car les vertices sont déjà en world space
		app.RegisterRender(tubeMesh, glm::mat4(1.0f), baseTexture.GetAllTextures());

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