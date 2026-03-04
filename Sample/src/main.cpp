#include <filesystem>
#include <iostream>
#include "Core/CameraComponent.h"
#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/LightComponent.h"
#include "Core/Spline.h"
#include "Core/Frenet.h"
#include "Core/Texture.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/Entities.h"
// to move 
struct ControllerComponent {};



int main(int argc, char** argv)
{


	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();
	KGR::RenderWindow::Init();
	KGR::RenderWindow window{ {1000,1000},"test",projectRoot / "Ressources" };
	window.GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);
	using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;
	auto registry = ecsType{};
	// Cam
	{
		auto cam = registry.CreateEntity();
		CameraComponent camComp = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0,3,5 });
		registry.AddComponents<CameraComponent, TransformComponent, ControllerComponent>(cam, std::move(camComp), std::move(transform), std::move(ControllerComponent{}));
	}

	// entity
	{
		auto mesh = registry.CreateEntity();
		MeshComponent meshComp;
		meshComp.mesh = &MeshLoader::Load("Models\\briet_claire_decorsfantasy_grpB.obj", window.App());
		TransformComponent transform;
		TextureComponent texture;
		texture.SetSize(meshComp.mesh->GetSubMeshesCount());
		for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
			texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));
		registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, ControllerComponent>(mesh, std::move(meshComp), std::move(transform), std::move(texture), std::move(ControllerComponent{}));
	}

	auto colorTransform = [](const glm::vec3& color)
		{
			glm::vec3 result;
			result.x = color.x * 1 / 255;
			result.y = color.y * 1 / 255;
			result.y = color.y * 1 / 255;
			result.y = color.y * 1 / 255;
			result.z = color.z * 1 / 255;
			return result;
		};
	{
		auto light = registry.CreateEntity();
		auto lComp = LightComponent<LightData::Type::Directional>::Create(colorTransform({ 154,36,69 }), { 1,1,1 }, 1.0f);
		TransformComponent lTransform;
		lTransform.LookAt({ 0,-1,0 });
		registry.AddComponents<LightComponent<LightData::Type::Directional>, TransformComponent>(light, std::move(lComp), std::move(lTransform));
	}

	std::vector<glm::vec3> points{


		// --- boucle principale ---
		{  0.0f,  6.0f,  0.0f },   // P1
		{ -5.5f,  4.0f, -2.0f },   // P2
		{ -6.0f, -1.0f, -3.0f },   // P3
		{ -2.0f, -5.5f, -1.0f },   // P4
		{  3.5f, -4.5f,  2.5f },   // P5
		{  6.5f, -1.0f,  3.0f },   // P6
		{  6.0f,  2.5f,  2.0f },   // P7

	};

	HermitCurve curve = HermitCurve::FromPoints(points, 0);

	const float rmfStep = 0.001f;
	const int rmfSampleCount = static_cast<int>(curve.MaxT() / rmfStep) + 1;

	std::vector<glm::vec3> rmfPoints;
	rmfPoints.reserve(rmfSampleCount);

	for (int i = 0; i < rmfSampleCount; ++i)
		rmfPoints.push_back(curve.Compute(i * rmfStep));

	auto rmfForwardDirs = KGR::RMF::EstimateForwardDirs(rmfPoints);
	auto rmfFrames = KGR::RMF::BuildFrames(rmfPoints, rmfForwardDirs);

	static float curvesTest = 0.0f;
	do
	{
		// event
		KGR::RenderWindow::PollEvent();
		window.Update();
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
		float camY = 5.0f;
		float camZ = std::sin(angle) * radius;
		{
			auto es = registry.GetAllComponentsView<ControllerComponent, TransformComponent, CameraComponent>();
			auto* inputData = window.GetInputManager();
			for (auto& e : es)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				glm::vec3 dir = { 0.0f,0.0f,0.0f };
				if (inputData->IsKeyDown(KGR::Key::Z))
					dir.z = -1;
				if (inputData->IsKeyDown(KGR::Key::S))
					dir.z = 1;
				if (inputData->IsKeyDown(KGR::Key::Q))
					dir.x = -1;
				if (inputData->IsKeyDown(KGR::Key::D))
					dir.x = 1;
				if (inputData->IsKeyDown(KGR::SpecialKey::Space))
					dir.y = 1;
				if (inputData->IsKeyDown(KGR::SpecialKey::Shift))
					dir.y = -1;
				auto delta = inputData->GetMouseDelta();
				transform.RotateEuler<RotData::Orientation::Pitch>(-glm::radians(delta.y * deltaTime * 100));
				transform.RotateEuler<RotData::Orientation::Yaw>(-glm::radians(delta.x * deltaTime * 100));
				transform.Translate(dir * deltaTime);
			}
		}

		{
			auto es = registry.GetAllComponentsView<ControllerComponent, TransformComponent, MeshComponent, TransformComponent>();
			for (auto& e : es)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				transform.SetPosition(curve.Compute(curvesTest));
				int frameIndex = glm::clamp(static_cast<int>(curvesTest / rmfStep), 0, static_cast<int>(rmfFrames.size() - 1));
				transform.SetOrientation(glm::quatLookAt(rmfFrames[frameIndex].forward, rmfFrames[frameIndex].up));
			}
		}

		curvesTest += 0.001f;
		if (curvesTest > curve.MaxT())
			curvesTest = 0.0f;

		{
			auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (es.Size() != 1)
				throw std::runtime_error("need one and one cam");
			for (auto& e : es)
			{
				//registry.GetComponent<TransformComponent>(e).SetPosition({ camX, camY, camZ });
				//registry.GetComponent<TransformComponent>(e).LookAt({ 0.0f, 0.0f, 0.0f });
				registry.GetComponent<CameraComponent>(e).UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
				window.RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
			}
		}


		// Render Mesh
		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, TextureComponent>();

			for (auto& e : es)
				window.RegisterRender(registry.GetComponent<MeshComponent>(e), registry.GetComponent<TransformComponent>(e), registry.GetComponent<TextureComponent>(e));
		}

		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Point>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Point>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Spot>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Spot>>(e), registry.GetComponent<TransformComponent>(e));
		}

		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Directional>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Directional>>(e), registry.GetComponent<TransformComponent>(e));
		}
		window.Render({ 0.53f,0.81f,0.92f ,1.0f });
	} while (!window.ShouldClose());


	window.Destroy();
	KGR::RenderWindow::End();
}