#pragma once
#define NOMINMAX

#include <glm/glm.hpp>
#include "Core/TrasformComponent.h"
#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "CollisionSystem.h"

/**
 * @brief runtime state for the canon viewmodel entity.
 * posOffset.x = right, posOffset.y = down, posOffset.z = forward (camera-space).
 */
struct CanonState
{
    uint64_t entity = UINT64_MAX;
    glm::vec3 posOffset = { 0.42f, 1.0f, 0.95f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
};

/**
 * @brief spawns the canon entity from a pre-loaded GLBAsset.
 * @param state canon state to initialize.
 * @param registry ECS registry.
 * @param asset pre-loaded GLB asset.
 * @param neutrals shared fallback textures.
 * @param skin optional texture override.
 */
template<typename TRegistry>
void CanonInit(CanonState& state, TRegistry& registry, const KGR::GLB::GLBAsset& asset,
    const KGR::GLB::GLBNeutralTextures& neutrals, const KGR::GLB::GLBSkinOverride* skin = nullptr)
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
 * @param state canon state.
 * @param registry ECS registry.
 * @param playerPos player capsule center in world space.
 * @param front normalized camera forward vector.
 */
template<typename TRegistry>
void CanonUpdate(CanonState& state, TRegistry& registry, const glm::vec3& playerPos, const glm::vec3& front)
{
    if (state.entity == UINT64_MAX)
        return;

    glm::vec3 right = glm::cross(front, glm::vec3{ 0.0f, 1.0f, 0.0f });
    if (glm::dot(right, right) < 1e-6f)
        right = glm::vec3{ 1.0f, 0.0f, 0.0f };
    else
        right = glm::normalize(right);

    const glm::vec3 up = glm::normalize(glm::cross(right, front));
    const glm::vec3 eyePos = playerPos + glm::vec3(0.0f, FPSViewEyeHeight, 0.0f);

    const glm::vec3 canonPos = eyePos
        + front * state.posOffset.z
        + right * state.posOffset.x
        - up * state.posOffset.y;

    auto& tc = registry.template GetComponent<TransformComponent>(state.entity);
    tc.SetPosition(canonPos);
    tc.LookAtDir(front);
}