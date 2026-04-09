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

using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;

/** @brief returns the world position of the first PlayerTag entity in the KGR registry. */
glm::vec3 GetPlayerPosition(ecsType& registry);

/**
 * @brief casts a ray against every enemy AABB and damages those hit.
 * enemies whose HP drops to zero are killed after the loop.
 */
void LaserHitEnemies(ts::Scene& scene, const glm::vec3& origin, const glm::vec3& dir, float maxDist);

/** @brief renders every enemy in ts::Scene through the KGR render pipeline. */
void RenderEnemies(ts::Scene& scene, KGR::RenderWindow* window, float dt);

/** @brief renders every KGR entity that has a mesh, handling skeletal animation when present. */
void RenderKGREntities(ecsType& registry, KGR::RenderWindow* window, float dt);

/** @brief entry point for the game logic — called by the engine. */
void RunGame(std::unique_ptr<KGR::RenderWindow>& window);