#include <iostream>

#include "Game.h"
#include "Core/CameraComponent.h"
int main(int argc, char** argv)
{
    //Scene game{};
    //game.Init(argv[0]);
    //game.Run(KGR::Tools::Chrono<float>::Time::CreateFromValue(1.0f/60.0f));

	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();
	KGR::RenderWindow::Init();
	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920,1080 }, "test", projectRoot / "Ressources");
	window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

	ecsType registry = ecsType{};


	{
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f),window->GetSize().x,window->GetSize().y,0.01f,100.0f,CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(cam), std::move(transform));
	}


	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj",window->App());

		TextureComponent text;
		text.SetSize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
			text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

		TransformComponent transform;
		transform.SetPosition({ 0,0,0 });
		//transform.RotateQuat<>()
		transform.RotateQuat<RotData::Orientation::Pitch>(glm::radians(-90.0f));
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(text), std::move(transform));
	}

	{
		
		LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create({ 1,0,1 }, { 1,1,1 }, 10.0f,100.0f,glm::radians(5.0f),0.15f);
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ 0,-1,0 });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	while (!window->ShouldClose())
	{
		float actual = chrono.GetElapsedTime().AsSeconds();
		float dt = actual - current;
		current = actual;
		

		{
			auto es = registry.GetAllComponentsView<MeshComponent,TransformComponent>();
			for (auto& e : es)
			{
				registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Yaw>(glm::radians(10.0f * dt));
			}

		}

		KGR::RenderWindow::PollEvent();
		window->Update();

		{
			auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (es.Size() != 1)
				throw std::runtime_error("need one and one cam");
			for (auto& e : es)
			{
				registry.GetComponent<CameraComponent>(e).UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
				registry.GetComponent<CameraComponent>(e).SetAspect(window->GetSize().x, window->GetSize().y);
				window->RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
			}
		}


		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, TextureComponent>();
			for (auto& e : es)
			{
				window->RegisterRender(
					registry.GetComponent<MeshComponent>(e),
					registry.GetComponent<TransformComponent>(e),
					registry.GetComponent<TextureComponent>(e));

				auto& t = registry.GetComponent<TransformComponent>(e);
				std::cout << t.GetRotation().x << " " << t.GetRotation().y << " " << t.GetRotation().z << std::endl;
			}

		}


		

		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Point>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Point>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Spot>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Spot>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Directional>, TransformComponent>();
			for (auto& e : es)
				window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Directional>>(e), registry.GetComponent<TransformComponent>(e));
		}

		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });


	}


	window->Destroy();
	KGR::RenderWindow::End();
}