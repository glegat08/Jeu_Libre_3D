#ifndef SPAWN_ENEMIES_H
#define SPAWN_ENEMIES_H

#include <random>
#include <numbers>
#include <glm/vec3.hpp>

#include "ts_ecs.h"
#include "Core/Window.h"
#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "CollisionSystem.h"
#include "EnemiesBehaviour.h"

/** @brief defines a circular area where enemies can be spawned. */
struct SpawnZone
{
    glm::vec3 center;
    float radius;
};

struct EnemyComponent {};
struct HealtComponent { int Health; };

/**
 * @brief spawns a single enemy from a pre-loaded GLB asset at the given position.
 * @param scene ts::Scene to spawn into.
 * @param asset pre-loaded GLB asset from GLBCache::Get().
 * @param neutrals shared 1x1 fallback textures for absent PBR channels.
 * @param pos world-space spawn position.
 * @param radius patrol radius passed to the Patrol action.
 * @param skin optional per-channel texture overrides.
 * @return the spawned entity handle.
 */
inline ts::Entity SpawnEnemy(ts::Scene& scene, const KGR::GLB::GLBAsset& asset,
    const KGR::GLB::GLBNeutralTextures& neutrals, glm::vec3 pos, float radius,
    const KGR::GLB::GLBSkinOverride* skin = nullptr)
{
    auto result = KGR::GLB::CreateGLBEntity(scene, asset, pos,
        glm::vec3{ 0.0f }, glm::vec3{ 4.0f }, neutrals, skin);

    if (!result.valid)
        return ts::Entity{};

    AIComponent ai;
    ai.m_ActionLists.push_back(Patrol(pos, radius));

    scene.Add<AIComponent>(result.entity, std::move(ai));
    scene.Add<EnemyComponent>(result.entity, EnemyComponent{});
    scene.Add<HealtComponent>(result.entity, HealtComponent{ 20 });
    scene.Add<PhysicsComponent>(result.entity, PhysicsComponent{});

    return result.entity;
}

/**
 * @brief tries to spawn one enemy inside the given zone, up to a max of 20 enemies in the scene.
 * @param scene ts::Scene to spawn into.
 * @param asset shared GLB asset — all enemies share the same mesh and textures.
 * @param neutrals shared fallback textures from GLBCache::GetNeutrals().
 * @param zone center and radius of the spawn area.
 * @param skin optional per-channel texture overrides.
 */
inline void SpawnEnemies(ts::Scene& scene, const KGR::GLB::GLBAsset& asset,
    const KGR::GLB::GLBNeutralTextures& neutrals, const SpawnZone& zone,
    const KGR::GLB::GLBSkinOverride* skin = nullptr)
{
    if (scene.Count<EnemyComponent>() >= 20)
        return;

    std::mt19937 gen{ std::random_device{}() };
    std::uniform_real_distribution<float> theta(0.0f, std::numbers::pi_v<float> *2.0f);
    std::uniform_real_distribution<float> r(0.0f, zone.radius);

    const glm::vec3 pos
    {
        zone.center.x + r(gen) * cosf(theta(gen)),
        zone.center.y,
        zone.center.z + r(gen) * sinf(theta(gen)),
    };

    SpawnEnemy(scene, asset, neutrals, pos, zone.radius, skin);
}

#endif