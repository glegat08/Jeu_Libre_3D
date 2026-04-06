#ifndef SPAWN_ENEMIES_H
#define SPAWN_ENEMIES_H

#include <random>
#include <numbers>
#include <glm/vec3.hpp>

#include "ts_ecs.h"
#include "Core/Window.h"
#include "EnemiesBehaviour.h"

struct SpawnZone
{
	glm::vec3 center;
	float radius;
};

struct EnemyComponent {};
struct HealtComponent { int Health; };

ts::Entity SpawnEnemy(const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene, const std::string& meshPath, const std::string& texturePath, glm::vec3 pos, float radius)
{
	auto mesh = MeshComponent();
	auto texture = TextureComponent();
	auto transform = TransformComponent();
	mesh.mesh = &MeshLoader::Load(meshPath, window->App());
	
	texture.texture = &TextureLoader::Load(texturePath, window->App());

	transform.SetScale({ 3.0f,3.0f,3.0f });
	transform.SetPosition(pos);

	AIComponent ai;
	ai.m_ActionLists.push_back(Patrol(pos, radius));

	return scene.Spawn(std::move(mesh), std::move(texture), std::move(transform),std::move(ai), EnemyComponent{}, HealtComponent{ 20 });
}
void SpawnEnemies(const std::unique_ptr<KGR::RenderWindow>& window, ts::Scene& scene, const SpawnZone& Spawn)
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

	switch (type(gen))
	{
	case 1:
		SpawnEnemy(window, scene, "Models/CUBE.obj", "Textures/BaseTexture.png", pos,Spawn.radius);
		break;
	case 2:
		SpawnEnemy(window, scene, "Models/monkey.obj", "Textures/BaseTexture.png", pos, Spawn.radius);
		break;
	case 0:
		SpawnEnemy(window, scene, "Models/stormtrooper.obj", "Textures/BaseTexture.png", pos, Spawn.radius);
		break;
	default:
		throw std::exception("Problem Spawn");
	}

}

#endif
