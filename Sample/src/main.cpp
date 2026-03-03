#include <filesystem>
#include <iostream>
#include "Core/CameraComponent.h"
#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/LightComponent.h"
#include "Core/Texture.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/Entities.h"
#include "GameFiles.h"

// to move 
struct ControllerComponent
{
};


struct IAComp
{
	
	void UpdateTarget(const glm::vec3& target)
	{
		m_target = target;
	}
	glm::vec3  Update(float dt,const  glm::vec3& pos)
	{
		glm::vec3 result = pos;
		glm::vec3 dir = glm::normalize(m_target - pos);
		result += dir * dt;
		return result;
	}

private :
	glm::vec3 m_target;
};




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
	/*{
	auto cam = registry.CreateEntity();
	CameraComponent camComp = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);
	TransformComponent transform;
	transform.SetPosition({ 0,3,5 });
	registry.AddComponents<CameraComponent, TransformComponent, ControllerComponent>(cam, std::move(camComp), std::move(transform),std::move(ControllerComponent{}));
	}*/

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
		registry.AddComponents<MeshComponent, TransformComponent, TextureComponent,ControllerComponent>(mesh, std::move(meshComp), std::move(transform), std::move(texture),std::move(ControllerComponent{}));
	}

	//Player
	{
		auto player = registry.CreateEntity();

		//Mesh
		MeshComponent meshComp;
		meshComp.mesh = &MeshLoader::Load("Models\\CUBE.obj", window.App());

		//Camera
		CameraComponent camComp = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);

		//Transform
		TransformComponent camTransform;
		camTransform.SetPosition({ 0,3,5 });

		//Texture
		TextureComponent texture;
		texture.SetSize(meshComp.mesh->GetSubMeshesCount());
		for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
			texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));

		registry.AddComponents<MeshComponent, CameraComponent, TransformComponent, TextureComponent, ControllerComponent, PlayerComponent /*WeaponComponent*/ >
			(player, std::move(meshComp), std::move(camComp), std::move(camTransform), std::move(texture), ControllerComponent{}, PlayerComponent{});
	}

	//// IA
	//{

	//	auto ia = registry.CreateEntity();
	//	MeshComponent meshComp;
	//	meshComp.mesh = &MeshLoader::Load("Models\\CUBE.obj", window.App());
	//	TransformComponent transform;
	//	transform.SetPosition({ 10, 0, 10}); // starting pos
	//	TextureComponent texture;
	//	texture.SetSize(meshComp.mesh->GetSubMeshesCount());
	//	for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
	//		texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));
	//	IAComp iaC;
	//	iaC.UpdateTarget({ 0,0,0 }); //player pos

	//	registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, IAComp>(ia, std::move(meshComp), std::move(transform), std::move(texture), std::move(iaC));

	//}


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
		lTransform.LookAt({ 0,-1,0});
		registry.AddComponents<LightComponent<LightData::Type::Directional>, TransformComponent>(light, std::move(lComp), std::move(lTransform));
	}






	do
	{
		/// EVENT PAS TOUCHE
		KGR::RenderWindow::PollEvent();
		window.Update();
		// EVENT
		


		//Update PAS TOUCHE
		static auto lastTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		// Update

		
		{
			auto es = registry.GetAllComponentsView<ControllerComponent, TransformComponent,CameraComponent>();
			auto* inputData = window.GetInputManager();
			for (auto& e : es)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				glm::vec3 dir = {0.0f,0.0f,0.0f};
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

		// UpdateIa
		static float spawnTimer = 0.0f;
		spawnTimer -= deltaTime;

		//Player pos
		glm::vec3 playerPos{ 0.0f };
		{
			auto view = registry.GetAllComponentsView<PlayerComponent, TransformComponent>();
			for (auto e : view)
			{
				playerPos = registry.GetComponent<TransformComponent>(e).GetPosition();
				break;
			}
		}

		//Spawn enemy
		if (spawnTimer <= 0.0f)
		{
			spawnTimer = 5.0f;

			KGR::GameLib::AIComponent ai;
			ai.SpawnEnemy(registry, window, playerPos);
		}

		
		//Update enemy
		{

			auto view = registry.GetAllComponentsView<KGR::GameLib::AIComponent, TransformComponent, KGR::GameLib::EnemyComponent>();
			for (auto& e : view)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				auto& ia = registry.GetComponent<KGR::GameLib::AIComponent>(e);
				auto& enemy = registry.GetComponent<KGR::GameLib::EnemyComponent>(e);

				ia.UpdateTarget(playerPos);

				glm::vec3 newPos = ia.Update(deltaTime, transform.GetPosition(), enemy.speed);
				transform.SetPosition(newPos);

				float dist = glm::length(playerPos - newPos);

				enemy.timeSinceLastAttack -= deltaTime;
				if (dist <= enemy.attackRange && enemy.timeSinceLastAttack <= 0.0f)
				{
					enemy.timeSinceLastAttack = enemy.attackCooldown;
					std::cout << "Enemy attacks player!\n";
				}
			}


		}


	/// RENDER PAS TOUCHE 

	{	
		auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
		if (es.Size() != 1)
			throw std::runtime_error("need one and one cam");
		for (auto& e : es)
		{
			auto& cam = registry.GetComponent<CameraComponent>(e);
			cam.UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
			if (cam.GetWidth() != window.GetSize().x || cam.GetHeight() != window.GetSize().y)
			{
				cam.SetAspect(window.GetSize().x, window.GetSize().y);
			}
			window.RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
		}
	}
	{
		auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent,TextureComponent>();

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
		window.Render({0.53f,0.81f,0.92f ,1.0f});
	}
	while (!window.ShouldClose());


	 window.Destroy();
	 KGR::RenderWindow::End();
}

