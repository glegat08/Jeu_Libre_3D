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
#include "EnemiesBehaviour.h"
#include "Components.h"


struct SpawnZone
{
	glm::vec3 center;
	float radius;
};



void SpawnEnemy(
	const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene, KGR::GLB::GLBCache& glbCache, KGR::GLB::GLBNeutralTextures& neutral,
	const std::string& meshPath, const std::string& texturePath, glm::vec3 pos, float radius)
{
	ts::Entity enemy;
	float RadarRange = radius + 10.0f;
	float PatrolArea = radius * 5.0f;

	const KGR::GLB::GLBAsset* enemyAsset = glbCache.Get(meshPath, window->App());
	Texture& texture = TextureLoader::Load(texturePath, window->App());
	if (enemyAsset)
		enemy = KGR::GLB::CreateGLBEntity<ts::Scene>(scene, *enemyAsset, pos, glm::vec3{ 0.0f,0.0f,0.0f }, glm::vec3(1.0f), neutral, KGR::GLB::GLBSkinOverride{ .baseColor = &texture }).entity;
	
	AIComponent ai;
	ai.m_ActionLists.push_back(Attack(RadarComponent{ RadarRange }));
	ai.m_ActionLists.push_back(Patrol(pos, PatrolArea));

	scene.Add<AIComponent>(enemy, std::move(ai));
	scene.Add<EnemyComponent>(enemy, EnemyComponent{});
	scene.Add<HealtComponent>(enemy, HealtComponent{ 20 });
}

void SpawnEnemies(
	const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene, KGR::GLB::GLBCache& glbCache, KGR::GLB::GLBNeutralTextures& neutral,
	const SpawnZone& Spawn)
{
	if (scene.Query<EnemyComponent>().Count() >= 20)
		return;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> theta(0.0f, std::numbers::pi * 2.0f);
	std::uniform_real_distribution<float> r(0.0f, Spawn.radius);
	std::uniform_int_distribution type(0, 2);

	glm::vec3 pos
	{ Spawn.center.x + r(gen) * cosf(theta(gen)),
		Spawn.center.y,
		Spawn.center.z + r(gen) * sinf(theta(gen)),
	};

	std::string AssetMob_bois = "GLB_Text_1by1/Mobs/Mob_bois/Mob_bois_anim.glb";
	std::string AssetMob_liane = "GLB_Text_1by1/Mobs/Mob_liane/Mob_liane_anim.glb";
	std::string AssetMob_pierre = "GLB_Text_1by1/Mobs/Mob_pierre/Mob_pierre_anim.glb";

	std::string TexturesMob_bois = "GLB_Text_1by1/Mobs/Mob_bois/Mob_bois_text.png";
	std::string TexturesMob_liane = "GLB_Text_1by1/Mobs/Mob_liane/Mob_liane.png";
	std::string TexturesMob_pierre = "GLB_Text_1by1/Mobs/Mob_pierre/Mob_pierre.png";
	switch (type(gen))
	{
	case 0:
		SpawnEnemy(window, scene, glbCache, neutral, AssetMob_bois, TexturesMob_bois, pos, Spawn.radius);
		break;
	case 1:
		SpawnEnemy(window, scene, glbCache, neutral, AssetMob_liane, TexturesMob_liane, pos, Spawn.radius);
		break;
	case 2:
		SpawnEnemy(window, scene, glbCache, neutral, AssetMob_pierre, TexturesMob_pierre, pos, Spawn.radius);
		break;
	default:
		throw std::exception("Problem Spawn");
	}

}

#endif
