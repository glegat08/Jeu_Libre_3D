#pragma once
#define NOMINMAX

#include <iostream>
#include <glm/glm.hpp>
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "RayAABB.h"

struct LaserTag {};

struct LaserState
{
    uint64_t laserEntity  = UINT64_MAX; 
    float    maxDistance  = 20.0f;      
    bool     hitLastFrame = false;     
};


template<typename TRegistry>
void LaserInit(
    LaserState& state,
    TRegistry& registry,
    KGR::RenderWindow* window,
    const std::filesystem::path& root)
{
    MeshComponent laserMesh;
    laserMesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

    TextureComponent laserTex;
    laserTex.SetSize(laserMesh.mesh->GetSubMeshesCount());
    for (int i = 0; i < (int)laserMesh.mesh->GetSubMeshesCount(); ++i)
        laserTex.AddTexture(i, &TextureLoader::Load("Textures/texture.jpg", window->App()));

    TransformComponent laserTransform;
    laserTransform.SetPosition({ 0, -9999, 0 }); 
    laserTransform.SetScale({ 0.05f, 0.05f, 1.0f });

    LaserTag tag;
    state.laserEntity = registry.CreateEntity();
    registry.AddComponents(state.laserEntity, std::move(laserMesh), std::move(laserTex), std::move(laserTransform), std::move(tag));
}


template<typename TRegistry>
void LaserUpdate(
    LaserState& state,
    TRegistry& registry,
    KGR::RenderWindow* window,
    const glm::vec3& playerPos,
    const glm::vec3& front)
{
    auto input      = window->GetInputManager();
    bool laserActive = input->IsMousePressed(KGR::Mouse::Button1);

    glm::vec3 laserEnd = playerPos + front * state.maxDistance;

    if (laserActive)
    {
        state.hitLastFrame = false;

        auto meshes = registry.template GetAllComponentsView<MeshComponent, TransformComponent>();
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == state.laserEntity) continue;

            MeshComponent&     mc = registry.template GetComponent<MeshComponent>(mesh);
            TransformComponent& tc = registry.template GetComponent<TransformComponent>(mesh);

            KGR::AABB3D aabb = ComputeWorldAABB(*mc.mesh, tc);

            if (RayAABB(playerPos, front, aabb, state.maxDistance))
            {
                state.hitLastFrame = true;
                std::cout << "[LASER] Object touched" << std::endl;

            }
        }
    }


    TransformComponent& laserTransform = registry.template GetComponent<TransformComponent>(state.laserEntity);

    if (laserActive)
    {
        float     laserLength = glm::distance(playerPos, laserEnd);
        glm::vec3 laserMid    = playerPos + front * (laserLength * 0.5f);

        laserTransform.SetPosition(laserMid);
        laserTransform.LookAtDir(front);
        laserTransform.SetScale({ 0.05f, 0.05f, laserLength * 0.5f });
    }
    else
    {
        laserTransform.SetPosition({ 0, -9999, 0 });
    }

    window->RegisterRender(
        registry.template GetComponent<MeshComponent>(state.laserEntity),
        registry.template GetComponent<TransformComponent>(state.laserEntity),
        registry.template GetComponent<TextureComponent>(state.laserEntity));
}
