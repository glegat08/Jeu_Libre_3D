#pragma once

#include <memory>
#include "Core/Window.h"
#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "Core/AnimationComponent.h"
#include "Core/LightComponent.h"

#include "FPSCamera.h"
#include "Laser.h"
#include "Canon.h"
#include "RayAABB.h"
#include "EnemiesBehaviour.h"
#include "Spawn_Enemies.h"
#include "UpdateLightComponents.h"
#include "ECS/Entities.h"

namespace ts
{
    class Scene;
}

/** @brief tags the player entity so FPSCameraUpdate can locate it in the KGR registry. */
struct PlayerTag {};

/** @brief counts down to zero then triggers removal of the owning entity. */
struct LifetimeComponent
{
    float remaining = 1.0f;
};

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

/** @brief returns the world position of the first PlayerTag entity in the KGR registry. */
glm::vec3 GetPlayerPosition(ecsType& registry);

/**
 * @brief casts a ray against every enemy AABB, damages those hit, and spawns a death effect
 *        in @p registry for enemies whose HP drops to zero.
 * @param scene ts::Scene containing enemies.
 * @param registry KGR registry used to spawn death effect entities.
 * @param origin ray origin in world space.
 * @param dir normalized ray direction.
 * @param maxDist maximum ray distance.
 * @param deathEffectMesh mesh used for the death effect billboard.
 * @param deathEffectMat material holding the death effect texture.
 */
void LaserHitEnemies(ts::Scene& scene, ecsType& registry,
    const glm::vec3& origin, const glm::vec3& dir, float maxDist,
    Mesh* deathEffectMesh, const MaterialComponent& deathEffectMat);

/** @brief renders every enemy in ts::Scene through the KGR render pipeline. */
void RenderEnemies(ts::Scene& scene, KGR::RenderWindow* window, float dt);

/** @brief renders every KGR entity that has a mesh, handling skeletal animation when present. */
void RenderKGREntities(ecsType& registry, KGR::RenderWindow* window, float dt);

/** @brief entry point for the game logic — called by the engine. */
void RunGame(std::unique_ptr<KGR::RenderWindow>& window);