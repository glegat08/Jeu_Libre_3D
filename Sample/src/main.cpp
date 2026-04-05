#include <iostream>

#include "Core/Transform2dComponent.h"
#include "Core/UiComponent.h"

#include "Core/InputManager.h"
#include "Core/CameraComponent.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Component.h"
#include "ECS/Entities.h"
#include "ECS/Registry.h"
#include "Tools/Chrono.h"
#include "Audio/SoundComponent.h"

struct MeshComponent2
{
	Mesh* mesh = nullptr;
	std::string sourcePath;
};

class TransformComponent2
{};


struct LaserTag {};


KGR::AABB3D ComputeWorldAABB(const Mesh& mesh, const TransformComponent& transform)
{
	glm::vec3 worldMin(std::numeric_limits<float>::max());
	glm::vec3 worldMax(-std::numeric_limits<float>::max());

	for (uint32_t i = 0; i < mesh.GetSubMeshesCount(); ++i)
	{
		for (const Vertex& v : mesh.GetSubMesh(i).GetVertices())
		{
			glm::vec3 worldPos = transform.GetPosition() + v.pos * transform.GetScale();
			worldMin = glm::min(worldMin, worldPos);
			worldMax = glm::max(worldMax, worldPos);
		}
	}

	return KGR::AABB3D(worldMin, worldMax);
}


bool RayAABB(const glm::vec3& origin, const glm::vec3& dir, const KGR::AABB3D& aabb, float maxDistance)
{
	glm::vec3 invDir = 1.0f / dir;

	glm::vec3 t1 = (aabb.GetMin() - origin) * invDir;
	glm::vec3 t2 = (aabb.GetMax() - origin) * invDir;

	glm::vec3 tMin = glm::min(t1, t2);
	glm::vec3 tMax = glm::max(t1, t2);

	float tEnter = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
	float tExit = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

	return tEnter <= tExit && tExit >= 0.0f && tEnter <= maxDistance;
}

// make you ecs type with entity 8 / 16 / 32 / 64 and the size of allocation between 1 and infinity
using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	KGR::RenderWindow::Init();
	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920,800 }, "test", projectRoot / "Ressources");

	window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

	ecsType registry = ecsType{};

	//MUSICS
	KGR::Audio::WavStreamComponent::Init(projectRoot / "Ressources");
	KGR::Audio::WavStreamComponent music;
	music.SetWav(KGR::Audio::WavStreamManager::Load("Musics/test.mp3"));
	music.SetVolume(10.0f);

	//SOUNDS
	KGR::Audio::WavComponent::Init(projectRoot / "Ressources");
	KGR::Audio::WavComponent sound;
	sound.SetWav(KGR::Audio::WavManager::Load("Sounds/sound.mp3"));
	sound.SetVolume(10.0f);

	music.Play();

	// camera
	{
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f), window->GetSize().x, window->GetSize().y, 0.01f, 100.0f, CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(cam), std::move(transform));
	}

	// mesh
	{
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		TextureComponent text;
		text.SetSize(mesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
			text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

		TransformComponent transform;
		transform.SetPosition({ 0,0,0 });
		transform.SetScale({ 2.0f,3.0f,4.0f });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(mesh), std::move(text), std::move(transform));
	}

	// player
	{
		MeshComponent2 player;
		player.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		TextureComponent text;
		text.SetSize(player.mesh->GetSubMeshesCount());
		for (int i = 0; i < player.mesh->GetSubMeshesCount(); ++i)
			text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

		TransformComponent transform;
		transform.SetPosition({ 5,0,0 });
		transform.SetScale({ 2.0f,3.0f,4.0f });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(player), std::move(text), std::move(transform));
	}

	// light
	{
		LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create({ 1,0,1 }, { 1,1,1 }, 10.0f, 100.0f, glm::radians(5.0f), 0.15f);
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ 0,-1,0 });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	// ui
	{
		TransformComponent2d transform;
		transform.SetRotation(glm::radians(-45.0f));
		UiComponent ui({ 1920,1080 }, UiComponent::Anchor::LeftTop);
		ui.SetPos({ 0, 0 });
		ui.SetScale({ 200,200 });
		TextureComponent texture;
		texture.SetSize(1);
		texture.AddTexture(0, &TextureLoader::Load("Textures/texture.jpg", window->App()));
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(transform), std::move(ui), std::move(texture));
	}

	auto laserEntity = registry.CreateEntity();
	{
		MeshComponent laserMesh;
		laserMesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

		TextureComponent laserTex;
		laserTex.SetSize(laserMesh.mesh->GetSubMeshesCount());
		for (int i = 0; i < laserMesh.mesh->GetSubMeshesCount(); ++i)
			laserTex.AddTexture(i, &TextureLoader::Load("Textures/texture.jpg", window->App()));

		TransformComponent laserTransform;
		laserTransform.SetPosition({ 0, -9999, 0 }); 
		laserTransform.SetScale({ 0.05f, 0.05f, 1.0f });

		LaserTag tag;
		registry.AddComponents(laserEntity, std::move(laserMesh), std::move(laserTex), std::move(laserTransform), std::move(tag));
	}


	static float laserMaxDistance = 20.0f;

	float yaw = -90.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.1f;
	float speed = 5.0f;

	bool isAttracting = false;
	bool isAttached = false;
	bool isFalling = false;
	float fallSpeed = 0.0f;
	float gravity = 9.81f;

	float current = 0.0f;
	KGR::Tools::Chrono<float> chrono;
	while (!window->ShouldClose())
	{
		float actual = chrono.GetElapsedTime().AsSeconds();
		float dt = actual - current;
		current = actual;

		{
			glm::vec2 mouseDelta = window->GetInputManager()->GetMouseDelta();
			yaw += mouseDelta.x * mouseSensitivity;
			pitch += mouseDelta.y * mouseSensitivity;
		}

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
		glm::vec3 right = glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)));

		glm::vec3 playerPos = { 0,0,0 };
		{
			auto players = registry.GetAllComponentsView<MeshComponent2, TransformComponent>();
			for (auto& player : players)
				playerPos = registry.GetComponent<TransformComponent>(player).GetPosition();
		}

		{
			auto es = registry.GetAllComponentsView<MeshComponent2, TransformComponent>();
			for (auto& e : es)
			{
				auto input = window->GetInputManager();
				TransformComponent& t = registry.GetComponent<TransformComponent>(e);

				if (input->IsKeyDown(KGR::Key::Z))
					t.Translate(flatFront * speed * dt);
				if (input->IsKeyDown(KGR::Key::S))
					t.Translate(-flatFront * speed * dt);
				if (input->IsKeyDown(KGR::Key::Q))
					t.Translate(-right * speed * dt);
				if (input->IsKeyDown(KGR::Key::D))
					t.Translate(right * speed * dt);
			}
		}

		{
			auto input = window->GetInputManager();
			if (input->IsMousePressed(KGR::Mouse::Button2))
			{
				if (isAttached)
				{
					isAttached = false;
					isFalling = true;
					fallSpeed = 0.0f;
				}
				else
				{
					isAttracting = !isAttracting;
				}
			}
		}

		if (isAttracting)
		{
			auto players = registry.GetAllComponentsView<MeshComponent2, TransformComponent>();
			auto meshes = registry.GetAllComponentsView<MeshComponent, TransformComponent>();

			for (auto& player : players)
			{
				glm::vec3 pPos = registry.GetComponent<TransformComponent>(player).GetPosition();

				for (auto& mesh : meshes)
				{
					if (mesh == laserEntity) continue;

					TransformComponent& meshTransform = registry.GetComponent<TransformComponent>(mesh);
					glm::vec3 meshPos = meshTransform.GetPosition();

					glm::vec3 direction = pPos - meshPos;
					float distance = glm::length(direction);

					glm::vec3 playerScale = registry.GetComponent<TransformComponent>(player).GetScale();
					glm::vec3 meshScale = meshTransform.GetScale();
					float contactDistance = (glm::length(playerScale) + glm::length(meshScale)) * 0.5f;

					if (distance > contactDistance)
					{
						glm::vec3 normalized = glm::normalize(direction);
						static float attractSpeed = 5.0f;
						meshTransform.Translate(normalized * attractSpeed * dt);
					}
					else
					{
						isAttracting = false;
						isAttached = true;
						isFalling = false;
						fallSpeed = 0.0f;
					}
				}
			}
		}

		if (isAttached)
		{
			auto meshes = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
			for (auto& mesh : meshes)
			{
				if (mesh == laserEntity) continue;
				registry.GetComponent<TransformComponent>(mesh).SetPosition(playerPos);
			}
		}

		if (isFalling)
		{
			auto meshes = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
			for (auto& mesh : meshes)
			{
				if (mesh == laserEntity) continue;
				TransformComponent& meshTransform = registry.GetComponent<TransformComponent>(mesh);
				fallSpeed += gravity * dt;
				meshTransform.Translate({ 0.0f, -fallSpeed * dt, 0.0f });

			}
		}

		{
			auto input = window->GetInputManager();
			bool laserActive = input->IsMousePressed(KGR::Mouse::Button1);

			glm::vec3 laserEnd = playerPos + front * laserMaxDistance;
			bool hitDetected = false;

			if (laserActive)
			{
				auto meshes = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
				for (auto& mesh : meshes)
				{
					if (mesh == laserEntity) continue;

					MeshComponent& mc = registry.GetComponent<MeshComponent>(mesh);
					TransformComponent& tc = registry.GetComponent<TransformComponent>(mesh);

					KGR::AABB3D aabb = ComputeWorldAABB(*mc.mesh, tc);

					if (RayAABB(playerPos, front, aabb, laserMaxDistance))
					{
						hitDetected = true;
						std::cout << "[LASER] Objet touché !" << std::endl;
					}
				}
			}

			TransformComponent& laserTransform = registry.GetComponent<TransformComponent>(laserEntity);

			if (laserActive)
			{
				float laserLength = glm::distance(playerPos, laserEnd);
				glm::vec3 laserMid = playerPos + front * (laserLength * 0.5f);

				laserTransform.SetPosition(laserMid);
				laserTransform.LookAtDir(front);
				laserTransform.SetScale({ 0.05f, 0.05f, laserLength * 0.5f });
			}
			else
			{
				laserTransform.SetPosition({ 0, -9999, 0 });
			}
		}

		KGR::RenderWindow::PollEvent();
		window->Update();

		// camera
		{
			auto cameras = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (cameras.Size() != 1)
				throw std::runtime_error("need one and one cam");

			for (auto& cam : cameras)
			{
				TransformComponent& camTransform = registry.GetComponent<TransformComponent>(cam);
				camTransform.SetPosition(playerPos);
				camTransform.LookAtDir(front);

				registry.GetComponent<CameraComponent>(cam).UpdateCamera(camTransform.GetFullTransform());
				registry.GetComponent<CameraComponent>(cam).SetAspect(window->GetSize().x, window->GetSize().y);
				window->RegisterCam(registry.GetComponent<CameraComponent>(cam), camTransform);
			}
		}

		// render mesh
		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, TextureComponent>();
			for (auto& e : es)
			{
				if (e == laserEntity) continue;
				window->RegisterRender(
					registry.GetComponent<MeshComponent>(e),
					registry.GetComponent<TransformComponent>(e),
					registry.GetComponent<TextureComponent>(e));
			}
		}

		// render laser
		{
			window->RegisterRender(
				registry.GetComponent<MeshComponent>(laserEntity),
				registry.GetComponent<TransformComponent>(laserEntity),
				registry.GetComponent<TextureComponent>(laserEntity));
		}

		// render player
		{
			auto es = registry.GetAllComponentsView<MeshComponent2, TransformComponent, TextureComponent>();
			for (auto& e : es)
			{
				MeshComponent tempMesh;
				tempMesh.mesh = registry.GetComponent<MeshComponent2>(e).mesh;
				window->RegisterRender(
					tempMesh,
					registry.GetComponent<TransformComponent>(e),
					registry.GetComponent<TextureComponent>(e));
			}
		}

		//Test Sound
		if (window->GetInputManager()->IsKeyPressed(KGR::Key::P))
			sound.Play();

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
		{
			auto es = registry.GetAllComponentsView<TextureComponent, TransformComponent2d, UiComponent>();
			for (auto& e : es)
			{
				auto transform = registry.GetComponent<TransformComponent2d>(e);
				auto ui = registry.GetComponent<UiComponent>(e);
				auto texture = registry.GetComponent<TextureComponent>(e);
				window->RegisterUi(ui, transform, texture);
			}
		}
		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
	}

	window->Destroy();
	KGR::RenderWindow::End();
}