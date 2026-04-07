#pragma once
#define NOMINMAX

#include <glm/glm.hpp>
#include "Core/TrasformComponent.h"
#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "Core/Window.h"
#include "ECS/Registry.h"

/**
 * @brief runtime state for the canon viewmodel entity.
 * posOffset.x = right, posOffset.y = down, posOffset.z = forward (camera-space).
 */
struct CanonState
{
    uint64_t entity = UINT64_MAX;
    glm::vec3 posOffset = { 0.f, 0.5f, 2.f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
};

/**
 * @brief spawns the canon entity from a pre-loaded GLBAsset.
 * @param skin optional texture override — pass nullptr to keep the GLB's own materials.
 */
template<typename TRegistry>
void CanonInit(CanonState& state, TRegistry& registry, const KGR::GLB::GLBAsset& asset, const KGR::GLB::GLBNeutralTextures& neutrals, const KGR::GLB::GLBSkinOverride* skin = nullptr)
{
    auto result = KGR::GLB::CreateGLBEntity(
        registry, asset,
        glm::vec3{ 0.0f }, glm::vec3{ 0.0f }, state.scale,
        neutrals, skin);

    if (result.valid)
        state.entity = static_cast<uint64_t>(result.entity);
}

/**
 * @brief repositions the canon every frame in camera space.
 * Call this after FPSCameraUpdate so that front is already up-to-date.
 */
template<typename TRegistry>
void CanonUpdate(CanonState& state, TRegistry& registry, const glm::vec3& playerPos, const glm::vec3& front)
{
    if (state.entity == UINT64_MAX)
        return;

    const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3{ 0.0f, 1.0f, 0.0f }));
    const glm::vec3 up = glm::normalize(glm::cross(right, front));

    const glm::vec3 canonPos = playerPos
        + front * state.posOffset.z
        + right * state.posOffset.x
        - up * state.posOffset.y;

    auto& tc = registry.template GetComponent<TransformComponent>(state.entity);
    tc.SetPosition(canonPos);
    tc.LookAtDir(front);
}