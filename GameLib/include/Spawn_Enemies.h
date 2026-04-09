#ifndef SPAWN_ENEMIES_H
#define SPAWN_ENEMIES_H

#include <random>
#include <numbers>
#include <glm/vec3.hpp>

#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "Core/AnimationComponent.h"
#include "ts_ecs.h"
#include "Core/Window.h"
#include "Core/GLBLoader.h"
#include "Core/GLBEntityFactory.h"
#include "EnemiesBehaviour.h"
#include "Components.h"


/** @brief Defines a circular area where enemies can be spawned. */
struct SpawnZone
{
	glm::vec3 center;
	float radius;
};


/**
 * @brief Spawns a single enemy from a pre-loaded GLB asset at the given position.
 * Uses MaterialComponent to stay compatible with the GLB render pipeline.
 * @param scene ts::Scene to spawn into.
 * @param asset Pre-loaded GLB asset from GLBCache::Get().
 * @param neutrals Shared 1x1 fallback textures for absent PBR channels.
 * @param pos World-space spawn position.
 * @param radius Patrol radius passed to the Patrol action.
 */
inline ts::Entity SpawnEnemy(ts::Scene& scene, const KGR::GLB::GLBAsset& asset, const KGR::GLB::GLBNeutralTextures& neutrals, glm::vec3 pos, float radius, const KGR::GLB::GLBSkinOverride* skin = nullptr)
{
	float RadarRange = radius + 10.0f;
	float PatrolArea = radius * 5.0f;
    MeshComponent mesh;
    mesh.mesh = asset.mesh.get();

    MaterialComponent mat = KGR::GLB::Detail::BuildMaterialComponent(
        asset, static_cast<int>(mesh.mesh->GetSubMeshesCount()), neutrals);

    if (skin)
        KGR::GLB::Detail::ApplySkinOverride(mat, *skin);

    TransformComponent transform;
    transform.SetPosition(pos);
    transform.SetScale({ 1.0f, 1.0f, 1.0f });

	RadarComponent Radar = { RadarRange };

    AIComponent ai;
    ai.m_ActionLists.push_back(Attack(Radar));
	ai.m_ActionLists.push_back(Patrol(pos, PatrolArea));

    return scene.Spawn(std::move(mesh), std::move(mat), std::move(transform), std::move(ai),std::move(Radar), EnemyComponent{}, HealtComponent{ 20 });
}

/**
 * @brief Tries to spawn one enemy inside the given zone, up to a max of 20 enemies in the scene.
 * @param scene ts::Scene to spawn into.
 * @param asset Shared GLB asset — all enemies share the same mesh and textures.
 * @param neutrals Shared fallback textures from GLBCache::GetNeutrals().
 * @param zone Center and radius of the spawn area.
 */
inline void SpawnEnemies(ts::Scene& scene, const KGR::GLB::GLBAsset& asset, const KGR::GLB::GLBNeutralTextures& neutrals, const SpawnZone& zone, const KGR::GLB::GLBSkinOverride* skin = nullptr)
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