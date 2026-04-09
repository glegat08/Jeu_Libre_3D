#pragma once
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"

#include "ECS/Registry.h"

struct MeshComponent2
{
    Mesh* mesh = nullptr;
    std::string sourcePath;
};

template<typename Registry>
void CreatePlayer(Registry& registry, KGR::RenderWindow* window)
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