#define NOMINMAX
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

#include "FPSCamera.h"
#include "Attraction.h"
#include "Laser.h"

// player
struct MeshComponent2
{
    Mesh* mesh = nullptr;
    std::string sourcePath;
};

class TransformComponent2 {};

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

int main(int argc, char** argv)
{
    std::filesystem::path exePath = argv[0];
    std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();

    KGR::RenderWindow::Init();
    std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(
        glm::vec2{ 1920, 800 }, "test", projectRoot / "Ressources");

    window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

    ecsType registry = ecsType{};


    // Audio
    KGR::Audio::WavStreamComponent::Init(projectRoot / "Ressources");
    KGR::Audio::WavStreamComponent music;
    music.SetWav(KGR::Audio::WavStreamManager::Load("Musics/test.mp3"));
    music.SetVolume(10.0f);

    KGR::Audio::WavComponent::Init(projectRoot / "Ressources");
    KGR::Audio::WavComponent sound;
    sound.SetWav(KGR::Audio::WavManager::Load("Sounds/sound.mp3"));
    sound.SetVolume(10.0f);

    music.Play();

    // Entities

    // camera
    {
        CameraComponent cam = CameraComponent::Create(
            glm::radians(45.0f), window->GetSize().x, window->GetSize().y,
            0.01f, 100.0f, CameraComponent::Type::Perspective);
        TransformComponent transform;
        transform.SetPosition({ 0, 3, 5 });
        transform.LookAt({ 0, 0, 0 });
        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(cam), std::move(transform));
    }

    // mesh
    {
        MeshComponent mesh;
        mesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

        TextureComponent text;
        text.SetSize(mesh.mesh->GetSubMeshesCount());
        for (int i = 0; i < (int)mesh.mesh->GetSubMeshesCount(); ++i)
            text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

        TransformComponent transform;
        transform.SetPosition({ 0, 0, 0 });
        transform.SetScale({ 2.0f, 3.0f, 4.0f });
        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(mesh), std::move(text), std::move(transform));
    }

    // player
    {
        MeshComponent2 player;
        player.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

        TextureComponent text;
        text.SetSize(player.mesh->GetSubMeshesCount());
        for (int i = 0; i < (int)player.mesh->GetSubMeshesCount(); ++i)
            text.AddTexture(i, &TextureLoader::Load("Textures/viking_room.png", window->App()));

        TransformComponent transform;
        transform.SetPosition({ 5, 0, 0 });
        transform.SetScale({ 2.0f, 3.0f, 4.0f });
        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(player), std::move(text), std::move(transform));
    }

    // light
    {
        LightComponent<LightData::Type::Spot> lc = LightComponent<LightData::Type::Spot>::Create(
            { 1,0,1 }, { 1,1,1 }, 10.0f, 100.0f, glm::radians(5.0f), 0.15f);
        TransformComponent transform;
        transform.SetPosition({ 0, 5, 0 });
        transform.LookAtDir({ 0, -1, 0 });
        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(lc), std::move(transform));
    }

    // UI
    {
        TransformComponent2d transform;
        transform.SetRotation(glm::radians(-45.0f));
        UiComponent ui({ 1920, 1080 }, UiComponent::Anchor::LeftTop);
        ui.SetPos({ 0, 0 });
        ui.SetScale({ 200, 200 });
        TextureComponent texture;
        texture.SetSize(1);
        texture.AddTexture(0, &TextureLoader::Load("Textures/texture.jpg", window->App()));
        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(transform), std::move(ui), std::move(texture));
    }


    FPSCameraState  cameraState;
    AttractionState attractionState;
    LaserState      laserState;

    LaserInit(laserState, registry, window.get(), projectRoot);


    float current = 0.0f;
    KGR::Tools::Chrono<float> chrono;

    while (!window->ShouldClose())
    {
        float actual = chrono.GetElapsedTime().AsSeconds();
        float dt = actual - current;
        current = actual;


        glm::vec3 front = { 0, 0, -1 };

        glm::vec3 playerPos = { 0, 0, 0 };
        {
            auto players = registry.GetAllComponentsView<MeshComponent2, TransformComponent>();
            for (auto& p : players)
                playerPos = registry.GetComponent<TransformComponent>(p).GetPosition();
        }


        FPSCameraUpdate<ecsType, MeshComponent2>(cameraState, registry, window.get(), dt, front);
        AttractionUpdate<ecsType, MeshComponent2>(attractionState, registry, window.get(), dt, laserState.laserEntity);
        LaserUpdate(laserState, registry, window.get(), playerPos, front);

        KGR::RenderWindow::PollEvent();
        window->Update();

        // mesh renderer
        {
            auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, TextureComponent>();
            for (auto& e : es)
            {
                if ((uint64_t)e == laserState.laserEntity) continue;
                window->RegisterRender(
                    registry.GetComponent<MeshComponent>(e),
                    registry.GetComponent<TransformComponent>(e),
                    registry.GetComponent<TextureComponent>(e));
            }
        }

        // player renderer
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


        if (window->GetInputManager()->IsKeyPressed(KGR::Key::P))
            sound.Play();

        // light
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

        // UI
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
