#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Core/TrasformComponent.h"
#include "Math/CollisionComponent.h"
#include "Math/SAT.h"

// Half-extents du joueur approximé en AABB.
// x/z = rayon du corps (0.4m), y = demi-hauteur (0.9m pour ~1.8m de haut).
static constexpr glm::vec3 kPlayerHalfExtents = { 0.4f, 0.9f, 0.4f };

/** @brief builds the player AABB centered on its world position. */
inline KGR::AABB3D MakePlayerAABB(const glm::vec3& pos)
{
    return KGR::AABB3D(pos - kPlayerHalfExtents, pos + kPlayerHalfExtents);
}

/**
 * @brief returns the real world-space scale of a TransformComponent.
 * GetScale() returns scale/2 because SetScale stores other/2 internally.
 * multiplying by 2 recovers the intended visual scale.
 */
inline glm::vec3 GetRealScale(const TransformComponent& tc)
{
    return tc.GetScale() * 2.0f;
}

/**
 * @brief resolves player movement against all CollisionComp entities using MTV.
 * uses OBB to correctly handle rotated map objects (tilted rocks, trees, etc.).
 * re-evaluates the player AABB after each correction so stacked collisions
 * (e.g. player wedged between two trees) resolve properly.
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