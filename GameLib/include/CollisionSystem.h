#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ts_ecs.h"
#include "Core/TrasformComponent.h"
#include "Math/CollisionComponent.h"
#include "Math/SAT.h"
#include "RayAABB.h"

static constexpr glm::vec3 playerHalfExtents = { 0.4f, 0.9f, 0.4f };
static constexpr float gravity = 9.8f;
static constexpr float groundTolerance = 0.05f;
static constexpr float jumpImpulse = 5.0f;

// shared by FPSCamera, Canon and Laser so all three reference the same value.
static constexpr float FPSViewEyeHeight = 0.7f;

/** @brief stores vertical velocity and grounded state for gravity simulation. */
struct PhysicsComponent
{
    float velocityY = 0.0f;
    bool isGrounded = false;
};

/** @brief builds the player AABB centered on its world position. */
inline KGR::AABB3D MakePlayerAABB(const glm::vec3& pos)
{
    return KGR::AABB3D(pos - playerHalfExtents, pos + playerHalfExtents);
}

/**
 * @brief returns the real world-space scale of a TransformComponent.
 * GetScale() returns scale/2 because SetScale stores value/2 internally.
 * multiplying by 2 recovers the intended visual scale.
 */
inline glm::vec3 GetRealScale(const TransformComponent& tc)
{
    return tc.GetScale() * 2.0f;
}

/**
 * @brief resolves player movement against all CollisionComp entities using MTV.
 * @param registry ECS registry.
 * @param currentPos player position before movement.
 * @param desiredMove displacement to apply this frame.
 * @return corrected displacement that avoids penetrating any collider.
 */
template<typename TRegistry>
glm::vec3 ResolvePlayerMovement(TRegistry& registry, const glm::vec3& currentPos,
    const glm::vec3& desiredMove)
{
    glm::vec3 resolvedPos = currentPos + desiredMove;
    KGR::AABB3D playerBox = MakePlayerAABB(resolvedPos);

    auto decorView = registry.template
        GetAllComponentsView<CollisionComp, TransformComponent>();

    for (auto& e : decorView)
    {
        const auto& cc = registry.template GetComponent<CollisionComp>(e);
        const auto& tc = registry.template GetComponent<TransformComponent>(e);

        if (!cc.collider)
            continue;

        const glm::vec3 realScale = tc.GetScale();
        const glm::vec3 pos = tc.GetPosition();
        const glm::quat rot = tc.GetOrientation();

        const KGR::OBB3D decorOBB = cc.collider->ComputeGlobalOBB(realScale, pos, rot);
        const KGR::Collision3D hit = KGR::SeparatingAxisTheorem::CheckCollisionAABBvsOBB(playerBox, decorOBB);

        if (!hit.IsColliding())
            continue;

        resolvedPos -= hit.GetCollisionNormal() * hit.GetPenetration();
        playerBox = MakePlayerAABB(resolvedPos);
    }

    return resolvedPos - currentPos;
}

/**
 * @brief applies gravity and resolves ground contact for a single physics/transform pair.
 * @param phys physics component to update.
 * @param tc transform component to update.
 * @param terrainMesh terrain mesh used for ground height sampling.
 * @param terrainPos world position of the terrain entity.
 * @param terrainScale scale of the terrain entity from GetScale().
 * @param dt seconds since last frame.
 * @param entityHalfHeight half-height of the entity used to offset from ground.
 */
inline void ResolveGravity(PhysicsComponent& phys, TransformComponent& tc,
    const Mesh& terrainMesh, const glm::vec3& terrainPos, const glm::vec3& terrainScale,
    float dt, float entityHalfHeight)
{
    phys.velocityY -= gravity * dt;

    glm::vec3 pos = tc.GetPosition();
    pos.y += phys.velocityY * dt;

    const float groundY = SampleTerrainHeight(terrainMesh, terrainPos, terrainScale, pos.x, pos.z, pos.y);
    const float targetY = groundY + entityHalfHeight;

    if (pos.y < targetY - groundTolerance)
    {
        pos.y = targetY;
        phys.velocityY = 0.0f;
        phys.isGrounded = true;
    }
    else if (pos.y <= targetY + groundTolerance)
    {
        if (phys.velocityY <= 0.0f)
        {
            pos.y = targetY;
            phys.velocityY = 0.0f;
            phys.isGrounded = true;
        }
        else
        {
            phys.isGrounded = false;
        }
    }
    else
    {
        phys.isGrounded = false;
    }

    tc.SetPosition(pos);
}

/**
 * @brief applies gravity to all entities with PhysicsComponent in a KGR registry.
 * @param registry KGR ECS registry.
 * @param terrainMesh terrain mesh used for ground height sampling.
 * @param terrainPos world position of the terrain entity.
 * @param terrainScale scale of the terrain entity from GetScale().
 * @param dt seconds since last frame.
 * @param entityHalfHeight half-height of the entity used to offset from ground.
 */
template<typename TRegistry>
void ApplyGravityRegistry(TRegistry& registry, const Mesh& terrainMesh,
    const glm::vec3& terrainPos, const glm::vec3& terrainScale, float dt, float entityHalfHeight)
{
    auto view = registry.template GetAllComponentsView<PhysicsComponent, TransformComponent>();

    for (auto& e : view)
    {
        auto& phys = registry.template GetComponent<PhysicsComponent>(e);
        auto& tc = registry.template GetComponent<TransformComponent>(e);
        ResolveGravity(phys, tc, terrainMesh, terrainPos, terrainScale, dt, entityHalfHeight);
    }
}

/**
 * @brief applies gravity to all entities with PhysicsComponent in a ts::Scene.
 * @param scene ts ECS scene.
 * @param terrainMesh terrain mesh used for ground height sampling.
 * @param terrainPos world position of the terrain entity.
 * @param terrainScale scale of the terrain entity from GetScale().
 * @param dt seconds since last frame.
 * @param entityHalfHeight half-height of the entity used to offset from ground.
 */
inline void ApplyGravityScene(ts::Scene& scene, const Mesh& terrainMesh,
    const glm::vec3& terrainPos, const glm::vec3& terrainScale, float dt, float entityHalfHeight)
{
    scene.Query<PhysicsComponent, TransformComponent>()
        .Each([&](ts::Entity, PhysicsComponent& phys, TransformComponent& tc)
            {
                ResolveGravity(phys, tc, terrainMesh, terrainPos, terrainScale, dt, entityHalfHeight);
            });
}