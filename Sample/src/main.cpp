#include <iostream>
#include <filesystem>

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
#include "Math/Collision2d.h"

#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "Core/AnimationComponent.h"
#include "VulkanCore.h"

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	KGR::RenderWindow::Init();

	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920, 1080 }, "KGR Engine", projectRoot / "Ressources");

	window->GetInputManager()->SetMode(GLFW_CURSOR_NORMAL);

	ecsType registry = ecsType{};

	// init the cache and upload the four shared neutral textures to GPU
	KGR::GLB::GLBCache glbCache;
	glbCache.Init(window->App());
	KGR::GLB::GLBNeutralTextures neutrals = glbCache.GetNeutrals();

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

	// camera
	{
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f), window->GetSize().x, window->GetSize().y, 0.01f, 100.0f, CameraComponent::Type::Perspective);
		TransformComponent transform;
		transform.SetPosition({ 0, 3, 5 });
		transform.LookAt({ 0, 0, 0 });
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(cam), std::move(transform));
	}

	// GLB entities
	{
		const KGR::GLB::GLBAsset* foxAsset = glbCache.Get("Models/Fox.glb", window->App());
		if (foxAsset)
			KGR::GLB::CreateGLBEntity(registry, *foxAsset,
				glm::vec3{0.0f, 0.0f, 2.0f}, glm::vec3(0.0f), glm::vec3(0.02f), neutrals);

		const KGR::GLB::GLBAsset* mobAsset = glbCache.Get("Models/Mobs.glb", window->App());
		if (mobAsset)
		{
			Texture& skinRed = TextureLoader::Load("Textures/Mob1.png", window->App());
			Texture& skinPurple = TextureLoader::Load("Textures/Mob2.png", window->App());
			Texture& skinOrange = TextureLoader::Load("Textures/Mob3.png", window->App());

			KGR::GLB::CreateGLBEntity(registry, *mobAsset,
				glm::vec3{ -2.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f}, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinRed });

			KGR::GLB::CreateGLBEntity(registry, *mobAsset,
				glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f }, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinPurple });

			KGR::GLB::CreateGLBEntity(registry, *mobAsset,
				glm::vec3{ 2.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f }, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinOrange });
		}
	}

	// light
	{
		LightComponent < LightData::Type::Directional > lc = LightComponent<LightData::Type::Directional>::Create({ 100,100,100 }, { 0,-1,-0.5f }, 1.0f);
		TransformComponent transform;
		transform.SetPosition({ 0, 5, 0 });
		transform.LookAtDir({0, 0, 0});
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	// ui
	{
		TransformComponent2d transform;
		UiComponent ui({ 1920, 1080 }, UiComponent::Anchor::LeftTop);
		ui.SetPos({ 0, 0 });
		ui.SetScale({ 200, 200 });
		TextureComponent texture;
		texture.texture = &TextureLoader::Load("Textures/texture.jpg", window->App());

		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(transform), std::move(ui), std::move(texture), std::move(CollisionComp2d{}));
	}

	KGR::Tools::Chrono<float> chrono;

	while (!window->ShouldClose())
	{
		float dt = chrono.GetDeltaTime();

		KGR::RenderWindow::PollEvent();
		window->Update();

		{
			static constexpr std::array<KGR::Key, 10> animKeys =
			{
				KGR::Key::Num1, KGR::Key::Num2, KGR::Key::Num3, KGR::Key::Num4
			};

			auto input = window->GetInputManager();
			auto es = registry.GetAllComponentsView<KGR::Animation::AnimationComponent>();
			for (auto& e : es)
			{
				auto& anim = registry.GetComponent<KGR::Animation::AnimationComponent>(e);
				for (size_t i = 0; i < animKeys.size(); ++i)
				{
					if (input->IsKeyPressed(animKeys[i]))
						anim.SetClip(i);
				}
			}
		}

		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
			for (auto& e : es)
			{
				auto input = window->GetInputManager();
				static float speed = 25.0f;
				if (input->IsKeyDown(KGR::Key::Q))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Yaw>(glm::radians(speed * dt));
				if (input->IsKeyDown(KGR::Key::D))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Yaw>(glm::radians(-speed * dt));
				if (input->IsKeyDown(KGR::Key::Z))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Pitch>(glm::radians(-speed * dt));
				if (input->IsKeyDown(KGR::Key::S))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Pitch>(glm::radians(speed * dt));
				if (input->IsKeyDown(KGR::Key::A))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Roll>(glm::radians(-speed * dt));
				if (input->IsKeyDown(KGR::Key::E))
					registry.GetComponent<TransformComponent>(e).RotateQuat<RotData::Orientation::Roll>(glm::radians(speed * dt));
			}
		}

		{
			auto mousePos = window.get()->GetInputManager()->GetMousePosition();
			float aspectRatio = static_cast<float>(window->GetSize().x) / static_cast<float>(window->GetSize().y);
			auto mouseinAR = UiComponent::VrToNdc(mousePos, window->GetSize(), aspectRatio, false);

			auto es = registry.GetAllComponentsView<CollisionComp2d, UiComponent>();
			for (auto e : es)
			{
				auto& t = registry.GetComponent<CollisionComp2d>(e);
				auto& u = registry.GetComponent<UiComponent>(e);
				t.Update(u.GetPosNdc(aspectRatio), u.GetScaleNdc(aspectRatio));

				if (t.aabb.IsColliding(mouseinAR))
					u.SetColor({ 1, 0, 0, 1 });
				else
					u.SetColor({ 0, 1, 0, 1 });
			}
		}

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
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, MaterialComponent>();
			for (auto& e : es)
			{
				int boneOffset = -1;

				if (registry.HasComponent<KGR::Animation::AnimationComponent>(e))
				{
					auto& anim = registry.GetComponent<KGR::Animation::AnimationComponent>(e);
					anim.Update(dt);
					boneOffset = window->App()->RegisterBoneMatrices(anim.GetLastBoneMatrices());
				}

				window->App()->RegisterRender(
					*registry.GetComponent<MeshComponent>(e).mesh,
					registry.GetComponent<TransformComponent>(e).GetFullTransform(),
					registry.GetComponent<MaterialComponent>(e).GetAllMaterials(),
					boneOffset
				);
			}
		}

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