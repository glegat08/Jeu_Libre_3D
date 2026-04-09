#pragma once
#define NOMINMAX

#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "RayAABB.h"
#include "Core/Materials.h"
#include "Core/Texture.h"
#include "ts_ecs.h"

struct LaserTag {};

struct LaserState
{
    uint64_t laserEntity = UINT64_MAX; 
    float maxDistance = 20.0f;      
    bool hitLastFrame = false;     
};

//template<typename RegistryT>
//struct RegistryTraits
//{
//    using type = RegistryT::type;
//    static type CreateEntity(RegistryT& registry) { return registry.CreateEntity(); }
//
//    template<typename... Components>
//    static void AddComponents(RegistryT& registry, type entity, Components&&... components)
//    {
//        registry.AddComponents(entity, std::forward<Components>(components)...);
//    }
//};
//
//template<>
//struct RegistryTraits<ts::Scene>
//{
//    using type = ts::Entity;
//    static type CreateEntity(ts::Scene& scene) { return scene.Spawn(); }
//
//    template<typename... Components>
//    static void AddComponents(ts::Scene& scene, type entity, Components&&... components)
//    {
//        (scene.Add<Components>(entity, std::forward<Components>(components)), ...);
//    }
//};

template<typename TRegistry>
void LaserInit(LaserState& state, TRegistry& registry, KGR::RenderWindow* window, const std::filesystem::path& root)
{
    MeshComponent laserMesh;
    laserMesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

    static std::unique_ptr<Texture> sBaseColor;
    static std::unique_ptr<Texture> sNormal;
    static std::unique_ptr<Texture> sPbr;
    static std::unique_ptr<Texture> sEmissive;

    if (!sBaseColor)
    {
        static const uint8_t white[4] = { 255, 255, 255, 255 };
        static const uint8_t normal[4] = { 128, 128, 255, 255 };
        static const uint8_t pbr[4] = { 255, 128, 0, 255 };
        static const uint8_t black[4] = { 0, 0, 0, 255 };

        sBaseColor = LoadTextureRaw(white, 1, 1, window->App());
        sNormal = LoadTextureRaw(normal, 1, 1, window->App());
        sPbr = LoadTextureRaw(pbr, 1, 1, window->App());
        sEmissive = LoadTextureRaw(black, 1, 1, window->App());
    }

    MaterialComponent laserMaterial;
    laserMaterial.SetSize(laserMesh.mesh->GetSubMeshesCount());
    for (int i = 0; i < (int)laserMesh.mesh->GetSubMeshesCount(); ++i) 
    {
        Material mat;
        mat.baseColor = sBaseColor.get();
        mat.normalMap = sNormal.get();
        mat.pbrMap = sPbr.get();
        mat.emissive = sEmissive.get();
        laserMaterial.AddMaterial(i, mat);
    }

    TransformComponent laserTransform;
    laserTransform.SetPosition({ 0, -9999, 0 }); 
    laserTransform.SetScale({ 0.05f, 0.05f, 1.0f });

    LaserTag tag;
    
    state.laserEntity = registry.CreateEntity();
    registry.AddComponents(state.laserEntity, std::move(laserMesh), std::move(laserMaterial), std::move(laserTransform), std::move(tag));
}


template<typename TRegistry>
void LaserUpdate(LaserState& state, TRegistry& registry, KGR::RenderWindow* window, const glm::vec3& playerPos, const glm::vec3& front)
{
    auto input = window->GetInputManager();
    bool laserActive = input->IsMousePressed(KGR::Mouse::Button1);

    glm::vec3 laserEnd = playerPos + front * state.maxDistance;

    if (laserActive)
    {
        state.hitLastFrame = false;

        auto meshes = registry.template GetAllComponentsView<MeshComponent, TransformComponent>();
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == state.laserEntity) continue;

            MeshComponent& mc = registry.template GetComponent<MeshComponent>(mesh);
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

    window->RegisterRender(
        registry.template GetComponent<MeshComponent>(state.laserEntity),
        registry.template GetComponent<TransformComponent>(state.laserEntity),
        registry.template GetComponent<MaterialComponent>(state.laserEntity),
        -1);  // boneOffset: -1 indicates no skeletal animation
}
