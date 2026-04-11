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
#include "Canon.h"

struct LaserTag {};

struct LaserState
{
    uint64_t laserEntity = UINT64_MAX;
    float maxDistance = 50.0f;
    bool hitLastFrame = false;
    glm::vec3 eyePos = { 0.0f, 0.0f, 0.0f };
};

template<typename TRegistry>
void LaserInit(LaserState& state, TRegistry& registry, KGR::RenderWindow* window, const std::filesystem::path& root)
{
    MeshComponent laserMesh;
    laserMesh.mesh = &MeshLoader::Load("Models/cube.obj", window->App());

    static Texture* sBaseColor = nullptr;
    static std::unique_ptr<Texture> sNormalOwned;
    static std::unique_ptr<Texture> sPbrOwned;

    if (!sBaseColor)
    {
        static const uint8_t normal[4] = { 128, 128, 255, 255 };
        static const uint8_t pbr[4] = { 255, 128, 0, 255 };

        sBaseColor = &TextureLoader::Load("Textures/laser_3.png", window->App());
        sNormalOwned = LoadTextureRaw(normal, 1, 1, window->App());
        sPbrOwned = LoadTextureRaw(pbr, 1, 1, window->App());
    }

    MaterialComponent laserMaterial;
    laserMaterial.SetSize(laserMesh.mesh->GetSubMeshesCount());
    for (int i = 0; i < (int)laserMesh.mesh->GetSubMeshesCount(); ++i)
    {
        Material mat;
        mat.baseColor = sBaseColor;
        mat.normalMap = sNormalOwned.get();
        mat.pbrMap = sPbrOwned.get();
        mat.emissive = sBaseColor;
        laserMaterial.AddMaterial(i, mat);
    }

    TransformComponent laserTransform;
    laserTransform.SetPosition({ 0.0f, -9999.0f, 0.0f });
    laserTransform.SetScale({ 0.05f, 0.05f, 1.0f });

    state.laserEntity = registry.CreateEntity();
    registry.AddComponents(state.laserEntity,
        std::move(laserMesh),
        std::move(laserMaterial),
        std::move(laserTransform),
        LaserTag{});
}

template<typename TRegistry>
void LaserUpdate(LaserState& state, TRegistry& registry, KGR::RenderWindow* window,
    const glm::vec3& playerPos, const glm::vec3& front,
    const glm::vec3& weaponOffset = glm::vec3{ 0.28f, 0.22f, 0.95f },
    uint64_t canonEntity = UINT64_MAX)
{
    const bool laserActive = window->GetInputManager()->IsMouseDown(KGR::Mouse::Button1);

    const glm::vec3 eyePos = playerPos + glm::vec3(0.0f, FPSViewEyeHeight, 0.0f);
    state.eyePos = eyePos;

    if (laserActive)
    {
        state.hitLastFrame = false;

        auto meshes = registry.template GetAllComponentsView<MeshComponent, TransformComponent>();
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == state.laserEntity)
                continue;

            const MeshComponent& mc = registry.template GetComponent<MeshComponent>(mesh);
            const TransformComponent& tc = registry.template GetComponent<TransformComponent>(mesh);

            if (RayAABB(eyePos, front, ComputeWorldAABB(*mc.mesh, tc), state.maxDistance))
            {
                state.hitLastFrame = true;
                std::cout << "[LASER] Object touched\n";
            }
        }
    }

    TransformComponent& laserTransform = registry.template GetComponent<TransformComponent>(state.laserEntity);

    if (laserActive)
    {
        glm::vec3 laserOrigin;

        if (canonEntity != UINT64_MAX
            && registry.template HasComponent<TransformComponent>(canonEntity))
        {
            constexpr float muzzleForwardOffset = 0.70f;
            laserOrigin = registry.template GetComponent<TransformComponent>(canonEntity).GetPosition()
                + front * muzzleForwardOffset;
        }
        else
        {
            glm::vec3 right = glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f));
            if (glm::dot(right, right) < 1e-6f)
                right = glm::vec3{ 1.0f, 0.0f, 0.0f };
            else
                right = glm::normalize(right);

            const glm::vec3 up = glm::normalize(glm::cross(right, front));
            laserOrigin = eyePos + right * weaponOffset.x - up * weaponOffset.y + front * weaponOffset.z;
        }

        const float beamHalfLength = state.maxDistance * 0.25f;
        laserTransform.SetPosition(laserOrigin + front * beamHalfLength);
        laserTransform.LookAtDir(front);
        laserTransform.SetScale({ 0.1f, 0.1f, state.maxDistance * 0.5f });
    }
    else
    {
        laserTransform.SetPosition({ 0.0f, -9999.0f, 0.0f });
    }

    window->RegisterRender(
        registry.template GetComponent<MeshComponent>(state.laserEntity),
        registry.template GetComponent<TransformComponent>(state.laserEntity),
        registry.template GetComponent<MaterialComponent>(state.laserEntity),
        -1);
}