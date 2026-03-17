#include <iostream>

#include "Core/Transform2dComponent.h"
#include "Core/UiComponent.h"


#include "InputManager.h"
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

// make you ecs type with entity 8 / 16 / 32 / 64 and the size of allocation between 1 and infinity
using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

int main(int argc, char** argv)
{
	
	// this part is due to the archi of the code to retrieve the folder resources
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();


	// init the rendering system ( init glfw )
	KGR::RenderWindow::Init();
	// create your window with the size the name and the resources path
	std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(glm::vec2{ 1920,800 }, "test", projectRoot / "Ressources");

	// getInputManager retrieve our input system where you can have the mouse pos mouse delta key pressed ... and set the cursor mode 
	window->GetInputManager()->SetMode(GLFW_CURSOR_NORMAL);

	// create your ecs 
	ecsType registry = ecsType{};


	// This is how to use the sounds and music system
	// and place it somewhere in the code where you want to use it

	// TODO when all test ok move this into a proper place 

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

	// TODO play the music for test 
	music.Play();

	// music test do not mind
		

	// camera 
	{
		// a calera need a cameraComponent that can be orthographic or perspective and a transform

		// create the camera with the fov , the size of the window (must be updated ) and the far and near rendering and the mode 
		CameraComponent cam = CameraComponent::Create(glm::radians(45.0f),window->GetSize().x,window->GetSize().y,0.01f,100.0f,CameraComponent::Type::Perspective);
		TransformComponent transform;
		// create a transform and set pos and dir 
		transform.SetPosition({ 0,3,5 });
		transform.LookAt({ 0,0,0 });
		// now create an entity , an alias here std::uint64_t
		auto e = registry.CreateEntity();

		// now move the component into the ecs
		registry.AddComponents(e, std::move(cam), std::move(transform));
	}

	
	// mesh
	{
		// a mesh need a meshComponent a transform and a texture 

		// create a mesh and load it with the cash loader
		MeshComponent mesh;
		mesh.mesh = &MeshLoader::Load("Models/cube.obj",window->App());

		// create a texture 
		TextureComponent text;
		// allocate the size of the texture must be the same as the number of submeshes 
		text.SetSize(mesh.mesh->GetSubMeshesCount());
		// then fill the texture ( this system need to be refact but for now you need to do it like that
		for (int i = 0; i < mesh.mesh->GetSubMeshesCount(); ++i)
			text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

		// create the transform and set all the data
		TransformComponent transform;
		transform.SetPosition({ 0,0,0 });
		transform.SetScale({ 2.0f,3.0f,4.0f });
		// same create an entity / id
		auto e = registry.CreateEntity();
		// fill the component
		registry.AddComponents(e, std::move(mesh), std::move(text), std::move(transform));
	}

	// light
	{
		// the light need transform component and light component
		// all lights type have their own system to create them go in the file to understand
		LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create({ 1,0,1 }, { 1,1,1 }, 10.0f,100.0f,glm::radians(5.0f),0.15f);
		// set the transform but certain light need dir some position or both so just use what necessary 
		TransformComponent transform;
		transform.SetPosition({ 0,5,0 });
		transform.LookAtDir({ 0,-1,0 });
		// same 
		auto e = registry.CreateEntity();
		// same
		registry.AddComponents(e, std::move(lc), std::move(transform));
	}

	// ui ( not fully operational)
	{
		// you need texture transform and ui component
		// for the transform it only use for the rotation 
		TransformComponent2d transform;
		// here you can set a rotation ( ROTATION FROM THE CENTER OF THE MESH )
		transform.SetRotation(glm::radians(-45.0f));
		// create your ui with a virtual resolution and an anchor default center
		UiComponent ui({1920,1080},UiComponent::Anchor::LeftTop);
		// here set the position in the virtual resolution
		ui.SetPos({ 0, 0 });
		// here the scale
		ui.SetScale({ 200,200 });
		// create a texture but be aware that only the first texture in the component will be use 
		TextureComponent texture;
		texture.SetSize(1);
		texture.AddTexture(0, &TextureLoader::Load("Textures/texture.jpg", window->App()));
		
		// same as always 
		auto e = registry.CreateEntity();
		registry.AddComponents(e, std::move(transform), std::move(ui),std::move(texture));

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
			}

		}

		//Test Sound
		if(window->GetInputManager()->IsKeyPressed(KGR::Key::P))
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
			auto es = registry.GetAllComponentsView < TextureComponent, TransformComponent2d,UiComponent > ();
			for (auto& e : es)
				{
					auto transform = registry.GetComponent<TransformComponent2d>(e);
					auto ui = registry.GetComponent<UiComponent>(e);
					auto texture = registry.GetComponent<TextureComponent>(e);
					window->RegisterUi(ui,transform,texture);
				}
		}
		window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
	}

	window->Destroy();
	KGR::RenderWindow::End();
}