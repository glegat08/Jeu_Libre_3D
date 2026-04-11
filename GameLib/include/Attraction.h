#pragma once
#define NOMINMAX

#include <glm/glm.hpp>
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "ECS/Registry.h"

struct AttractionState
{
    bool  isAttracting = false;
    bool  isAttached   = false;
    bool  isFalling    = false;
    float fallSpeed    = 0.0f;
    float attractSpeed = 5.0f;
    float gravity      = 9.81f;
};

template<typename TRegistry, typename TPlayerTag>
void AttractionUpdate(
    AttractionState& state,
    TRegistry& registry,
    KGR::RenderWindow* window,
    float dt,
    uint64_t excludeEntity = UINT64_MAX)
{
    auto input = window->GetInputManager();

    if (input->IsMousePressed(KGR::Mouse::Button2))
    {
        if (state.isAttached)
        {
            state.isAttached   = false;
            state.isFalling    = true;
            state.fallSpeed    = 0.0f;
        }
        else
        {
            state.isAttracting = !state.isAttracting;
        }
    }

    glm::vec3 playerPos = { 0,0,0 };
    glm::vec3 playerScale = { 1,1,1 };
    {
        auto players = registry.template GetAllComponentsView<TPlayerTag, TransformComponent>();
        for (auto& p : players)
        {
            playerPos   = registry.template GetComponent<TransformComponent>(p).GetPosition();
            playerScale = registry.template GetComponent<TransformComponent>(p).GetScale();
        }
    }

    auto meshes = registry.template GetAllComponentsView<MeshComponent, TransformComponent>();

    if (state.isAttracting)
    {
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == excludeEntity) continue;

            TransformComponent& meshTransform = registry.template GetComponent<TransformComponent>(mesh);
            glm::vec3 meshPos   = meshTransform.GetPosition();
            glm::vec3 meshScale = meshTransform.GetScale();

            glm::vec3 direction       = playerPos - meshPos;
            float distance            = glm::length(direction);
            float contactDistance     = (glm::length(playerScale) + glm::length(meshScale)) * 0.5f;

            if (distance > contactDistance)
            {
                meshTransform.Translate(glm::normalize(direction) * state.attractSpeed * dt);
            }
            else
            {
                state.isAttracting = false;
                state.isAttached   = true;
                state.isFalling    = false;
                state.fallSpeed    = 0.0f;
            }
        }
    }

    if (state.isAttached)
    {
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == excludeEntity) continue;
            registry.template GetComponent<TransformComponent>(mesh).SetPosition(playerPos);
        }
    }


    if (state.isFalling)
    {
        for (auto& mesh : meshes)
        {
            if ((uint64_t)mesh == excludeEntity) continue;
            TransformComponent& meshTransform = registry.template GetComponent<TransformComponent>(mesh);
            state.fallSpeed += state.gravity * dt;
            meshTransform.Translate({ 0.0f, -state.fallSpeed * dt, 0.0f });
        }
    }
}
