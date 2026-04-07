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

#include"ts_ecs.h"
#include "Spawn_Enemies.h"
#include "UpdateLightComponents.h"
#include "EnemiesBehaviour.h"
#include "Create_Parcelles.h"




using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;



int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

	KGR::RenderWindow::Init();

	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920, 1080 }, "KGR Engine", projectRoot / "Ressources");

	// getInputManager retrieve our input system where you can have the mouse pos mouse delta key pressed ... and set the cursor mode 
	window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

	//ecsType registry = ecsType{};
	ts::Scene scene;

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
		//// a calera need a cameraComponent that can be orthographic or perspective and a transform

		//// create the camera with the fov , the size of the window (must be updated ) and the far and near rendering and the mode 
		//CameraComponent cam = CameraComponent::Create(glm::radians(45.0f),window->GetSize().x,window->GetSize().y,0.01f,100.0f,CameraComponent::Type::Perspective);
		//TransformComponent transform;
		//// create a transform and set pos and dir 
		//transform.SetPosition({ 0,3,5 });
		//transform.LookAt({ 0,0,0 });
		//// now create an entity , an alias here std::uint64_t
		//auto e = registry.CreateEntity();
		//
		//// now move the component into the ecs
		//registry.AddComponents(e, std::move(cam), std::move(transform));


		auto cam = scene.Spawn();
		scene.Add<CameraComponent>(std::move(cam), { CameraComponent::Create(glm::radians(45.0f),window->GetSize().x,window->GetSize().y,0.01f,1000.0f,CameraComponent::Type::Perspective) });
		TransformComponent transform;
		transform.SetPosition({ 0.0f,1.0f,0.0f });
		transform.LookAt({ 0.0f,0.0f,0.0f });
		transform.SetRotation(glm::vec3(0.0f));

		scene.Add<TransformComponent>(std::move(cam), std::move(transform));
	}

	// GLB entities
	{
		//scale map : glm::vec3{ 5.f, 5.f, 0.04f }
		const KGR::GLB::GLBAsset* Test = glbCache.Get("Models/Cube.glb", window->App());
		const KGR::GLB::GLBAsset* TestLight = glbCache.Get("Models/CesiumMan.glb", window->App());
		Texture& textureTest = TextureLoader::Load("Textures/BaseTexture.png", window->App());
		Texture& textureRepere = TextureLoader::Load("Textures/BaseTexture.png", window->App());
		Texture& textureRed = TextureLoader::Load("Textures/rouge.jpg", window->App());
		KGR::GLB::CreateGLBEntity<ts::Scene>(scene, *Test,
			glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3(500.0f, 1.0f, 500.0f),
			neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &textureTest });

		KGR::GLB::CreateGLBEntity<ts::Scene>(scene, *Test,
			glm::vec3{ 0.0f, 0.0f, 100.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3(50.0f),
			neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &textureRepere });

		KGR::GLB::CreateGLBEntity<ts::Scene>(scene, *Test,
			glm::vec3{ 100.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3(50.0f),
			neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &textureRed });

		/*KGR::GLB::CreateGLBEntity<ts::Scene>(scene, *TestLight,
			glm::vec3{ 1.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3(5.0f),
			neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &textureTest });*/

		

		/*const KGR::GLB::GLBAsset* mobAsset = glbCache.Get("Models/Mobs.glb", window->App());
		if (mobAsset)
		{
			Texture& skinRed = TextureLoader::Load("Textures/Mob1.png", window->App());
			Texture& skinPurple = TextureLoader::Load("Textures/Mob2.png", window->App());
			Texture& skinOrange = TextureLoader::Load("Textures/Mob3.png", window->App());

			KGR::GLB::CreateGLBEntity(scene, *mobAsset,
				glm::vec3{ -2.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f }, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinRed });

			KGR::GLB::CreateGLBEntity(scene, *mobAsset,
				glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f }, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinPurple });

			KGR::GLB::CreateGLBEntity(scene, *mobAsset,
				glm::vec3{ 2.0f, 0.0f, 0.0f }, glm::vec3{ 90.0f, 0.0f, 0.0f }, glm::vec3(1.0f),
				neutrals, KGR::GLB::GLBSkinOverride{ .baseColor = &skinOrange });
		}*/
	}
	/*Create_Parcelles(window, scene, glbCache, neutrals,
		"GLB_Text_1by1/Parcelle bois/Parcelle_bois.glb",
		"GLB_Text_1by1/Parcelle bois/Pacerelle Bois.png",
		glm::vec3{ -60.0f,0.0f,10.0f });
	Create_Parcelles(window, scene, glbCache, neutrals,
		"GLB_Text_1by1/Parcelle liane/Parcelle_liane.glb",
		"GLB_Text_1by1/Parcelle liane/Pacerelle Liane.png",
		glm::vec3{ 0.0f,0.0f,10.0f });
	Create_Parcelles(window, scene, glbCache, neutrals,
		"GLB_Text_1by1/Parcelle pierre/Parcelle_pierre.glb",
		"GLB_Text_1by1/Parcelle pierre/Pacerelle Pierre.png",
		glm::vec3{ 60.0f,0.0f,10.0f });*/

	//// mesh
	//{
	//	// a mesh need a meshComponent a transform and a texture 

	//	// create a mesh and load it with the cash loader
	//	MeshComponent tempo_map;
	//	tempo_map.mesh = &MeshLoader::Load("Models/cube.obj",window->App());

	//	//// create a texture 
	//	//TextureComponent text;
	//	//// allocate the size of the texture must be the same as the number of submeshes 
	//	//text.(tempo_map.mesh->GetSubMeshesCount());
	//	//// then fill the texture ( this system need to be refact but for now you need to do it like that
	//	//for (int i = 0; i < tempo_map.mesh->GetSubMeshesCount(); ++i)
	//	//	text.AddTexture(i, &TextureLoader::Load("Textures/BaseTexture.png", window->App()));
	//	TextureComponent text;
	//	text.texture = &TextureLoader::Load("Textures/BaseTexture.png", window->App());


	//	// create the transform and set all the data
	//	TransformComponent transform;
	//	transform.SetPosition({ 0.0f,0.0f,0.0f });
	//	transform.SetScale({ 100.0f,0.5f,100.0f });
	//	// same create an entity / id
	//	auto map = scene.Spawn();
	//	// fill the component
	//	//registry.AddComponents(e, std::move(tempo_map), std::move(text), std::move(transform));
	//	scene.Add<MeshComponent>(map, std::move(tempo_map));
	//	scene.Add<TextureComponent>(map, std::move(text));
	//	scene.Add<TransformComponent>(map, std::move(transform));
	//	scene.Add<MapComponent>(map, MapComponent());
	//	
	//}

	// light
	{
		//// the light need transform component and light component
		//// all lights type have their own system to create them go in the file to understand
		//LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create({ 1,0,1 }, { 1,1,1 }, 10.0f,100.0f,glm::radians(5.0f),0.15f);
		//LightComponent<LightData::Type::Directional> lightDirection = LightComponent<LightData::Type::Directional>::Create({ 1,1,1 }, { 0,0,0 }, 500.0f);
		//// set the transform but certain light need dir some position or both so just use what necessary 
		//TransformComponent transform;
		//transform.SetPosition({ 0,5,0 });
		//transform.LookAtDir({ 0,-1,0 });
		//transform.SetScale({ -3,-3,-3 });
		//// same 
		//auto e = registry.CreateEntity();
		//// same
		//registry.AddComponents(e, std::move(lightDirection), std::move(transform));

		LightComponent<LightData::Type::Directional> Sun = LightComponent<LightData::Type::Directional>::Create({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, 500.0f);
		TransformComponent transform;
		transform.SetPosition({ 0.0f,50.0f,0.0f });
		transform.LookAtDir({ 0.0f,-1.0f,0.0f });
		auto Sun_Entity = scene.Spawn();
		scene.Add(Sun_Entity, std::move(Sun));
		scene.Add(Sun_Entity, std::move(transform));


	}

	// ui
	{
		TransformComponent2d transform;
		UiComponent ui({ 1920, 1080 }, UiComponent::Anchor::LeftTop);
		ui.SetPos({ 0, 0 });
		ui.SetScale({ 200, 200 });
		TextureComponent texture;
		texture.texture = &TextureLoader::Load("Textures/texture.jpg", window->App());

		auto e = scene.Spawn();
		//registry.AddComponents(e, std::move(transform), std::move(ui), std::move(texture), std::move(CollisionComp2d{}));
		scene.Add(std::move(e), std::move(transform));
		scene.Add(std::move(e), std::move(ui));
		scene.Add(std::move(e), std::move(texture));
		scene.Add(std::move(e), CollisionComp2d{});
	}

	float current = 0.0f;
	float timer = 0.0f;
	bool HasValidFrame = false;
	char Count = 0;
	KGR::Tools::Chrono<float> chrono;

	while (!window->ShouldClose())
	{
		//TODO :VERIF DELTATIME AND COMBINAISON INPUT
		float dt = chrono.GetDeltaTime();

		KGR::RenderWindow::PollEvent();
		window->Update();

		/*timer += dt;
		if (timer >= 2.0f)
		{
			SpawnEnemies(window, scene, glbCache, neutrals, SpawnZone{ {0.0f,0.0f,10.0f},5.0f });
			timer = 0.0f;
		}*/

		{
			static constexpr std::array<KGR::Key, 10> animKeys =
			{
				KGR::Key::Num1, KGR::Key::Num2, KGR::Key::Num3, KGR::Key::Num4
			};

			auto input = window->GetInputManager();

			/*auto es = registry.GetAllComponentsView<KGR::Animation::AnimationComponent>();
			for (auto& e : es)
			{
				auto& anim = registry.GetComponent<KGR::Animation::AnimationComponent>(e);
				for (size_t i = 0; i < animKeys.size(); ++i)
				{
					if (input->IsKeyPressed(animKeys[i]))
						anim.SetClip(i);
				}
			}*/
			scene.Query<KGR::Animation::AnimationComponent>()
				.Each([&](ts::Entity e, KGR::Animation::AnimationComponent& animation)
					{
						for (size_t i = 0; i < animKeys.size(); ++i)
						{
							if (input->IsKeyPressed(animKeys[i]))
								animation.SetClip(i);
						}
					});


		}


		Count += 1;
		if (HasValidFrame)
		{
			/*auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent>();
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
			}*/


			//{
			//	auto mousePos = window.get()->GetInputManager()->GetMousePosition();
			//	float aspectRatio = static_cast<float>(window->GetSize().x) / static_cast<float>(window->GetSize().y);
			//	auto mouseinAR = UiComponent::VrToNdc(mousePos, window->GetSize(), aspectRatio, false);

			//	/*auto es = registry.GetAllComponentsView<CollisionComp2d, UiComponent>();
			//	for (auto e : es)
			//	{
			//		auto& t = registry.GetComponent<CollisionComp2d>(e);
			//		auto& u = registry.GetComponent<UiComponent>(e);
			//		t.Update(u.GetPosNdc(aspectRatio), u.GetScaleNdc(aspectRatio));

			//		if (t.aabb.IsColliding(mouseinAR))
			//			u.SetColor({ 1, 0, 0, 1 });
			//		else
			//			u.SetColor({ 0, 1, 0, 1 });
			//	}*/
			//	scene.Query<CollisionComp2d, UiComponent>()
			//		.Each([&](ts::Entity e, CollisionComp2d& boxColl, UiComponent& ui)
			//			{
			//				boxColl.Update(ui.GetPosNdc(aspectRatio), ui.GetScaleNdc(aspectRatio));
			//				if (boxColl.aabb.IsColliding(mouseinAR))
			//					ui.SetColor({ 1, 0, 0, 1 });
			//				else
			//					ui.SetColor({ 0, 1, 0, 1 });
			//			});
			//}


			scene.Query<CameraComponent, TransformComponent>()
				.Each([&](ts::Entity e, CameraComponent& cam, TransformComponent& transform)
					{
						auto input = window->GetInputManager();
						static float SpeedMovCam = 25.0f;

						//Allows you to move the camera using the Z, Q, S, and D keys
						if (input->IsKeyDown(KGR::Key::Q))
							transform.SetPosition(transform.GetPosition() + transform.GetLocalAxe<RotData::Dir::Left>() * SpeedMovCam * dt);
						if (input->IsKeyDown(KGR::Key::D))
							transform.SetPosition(transform.GetPosition() + transform.GetLocalAxe<RotData::Dir::Right>() * SpeedMovCam * dt);

						if (input->IsKeyDown(KGR::Key::Z))
							transform.SetPosition(transform.GetPosition() + transform.GetLocalAxe<RotData::Dir::Forward>() * SpeedMovCam * dt);
						if (input->IsKeyDown(KGR::Key::S))
							transform.SetPosition(transform.GetPosition() + transform.GetLocalAxe<RotData::Dir::Backward>() * SpeedMovCam * dt);

						static float SpeedRollCam = SpeedMovCam * 2.0f;
						if (input->IsKeyDown(KGR::Key::A))
							transform.RotateQuat<RotData::Orientation::Roll>(glm::radians(-SpeedRollCam * dt));
						if (input->IsKeyDown(KGR::Key::E))
							transform.RotateQuat<RotData::Orientation::Roll>(glm::radians(SpeedRollCam * dt));


						//Allows you to pan the camera using the mouse 
						static float MouseSensitivity = 1.0f;
						transform.RotateQuat<RotData::Orientation::Yaw>(-input->GetMouseDelta().x * MouseSensitivity * dt);
						transform.RotateQuat<RotData::Orientation::Pitch>(-input->GetMouseDelta().y * MouseSensitivity * dt);
					});
		}


		AIEnemiesSystem(scene, dt);

		/*KGR::RenderWindow::PollEvent();
		window->Update();*/

		{
			/*auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (es.Size() != 1)
				throw std::runtime_error("need one and one cam");
			for (auto& e : es)
			{
				registry.GetComponent<CameraComponent>(e).UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
				registry.GetComponent<CameraComponent>(e).SetAspect(window->GetSize().x, window->GetSize().y);
				window->RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
			}*/

			if (scene.Count<CameraComponent>() != 1)
				throw std::runtime_error("need one and one cam");

			scene.Query<CameraComponent, TransformComponent>()
				.Each([&](ts::Entity e, CameraComponent& cam, TransformComponent& transform)
					{
						cam.UpdateCamera(transform.GetFullTransform());
						cam.SetAspect(window->GetSize().x, window->GetSize().y);
						window->RegisterCam(cam, transform);
						//std::println(transform.)
					});
		}

		{
			/*auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, MaterialComponent>();
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
			}*/
			scene.Query<MeshComponent, TransformComponent, MaterialComponent>()
				.Each([&](ts::Entity e, MeshComponent& mesh, TransformComponent& transform, MaterialComponent& material)
					{
						int boneOffset = -1;
						if (auto anim = scene.GetComponent<KGR::Animation::AnimationComponent>(e))
						{
							anim->Update(dt);
							boneOffset = window->App()->RegisterBoneMatrices(anim->GetLastBoneMatrices());
						}
						window->App()->RegisterRender((*mesh.mesh), transform.GetFullTransform(), material.GetAllMaterials(), boneOffset);
					});

			/*scene.Query<MeshComponent, TransformComponent, TextureComponent>()
				.Where([&](const ts::Entity e, const MeshComponent& mesh, const TransformComponent& transform, const TextureComponent& texture)
					{
						return scene.HasComponent<EnemyComponent>(e);
					})
				.Each([&](ts::Entity e, MeshComponent& mesh, TransformComponent& transform, TextureComponent& texture) {window->RegisterRender(mesh, transform, texture); });*/
		}

		if (window->GetInputManager()->IsKeyPressed(KGR::Key::P))
			sound.Play();

		/*{
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
		}*/
		{
			UpdateLightComponents<LightData::Type::Directional>(window, scene);
			UpdateLightComponents<LightData::Type::Spot>(window, scene);
			UpdateLightComponents<LightData::Type::Point>(window, scene);
		}
		{
			/*auto es = registry.GetAllComponentsView<TextureComponent, TransformComponent2d, UiComponent>();
			for (auto& e : es)
			{
				auto transform = registry.GetComponent<TransformComponent2d>(e);
				auto ui = registry.GetComponent<UiComponent>(e);
				auto texture = registry.GetComponent<TextureComponent>(e);
				window->RegisterUi(ui, transform, texture);
			}*/

			scene.Query<TextureComponent, TransformComponent2d, UiComponent>()
				.Each([&](ts::Entity e, TextureComponent& texture, TransformComponent2d& transform, UiComponent& ui) {window->RegisterUi(ui, transform, texture); });
		}

		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
		if (Count > 5)
			HasValidFrame = true;
	}
	window->Destroy();
	KGR::RenderWindow::End();
}