#ifndef ENEMIESBEHAVIOUR_H
#define ENEMIESBEHAVIOUR_H

#include <random>
#include <numbers>
#include <functional>
#include <any>

#include "ts_ecs.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"

struct EnemyComponent;

//TODO : COMMENTEZ 
//TODO : Changer le mouvement pour un slime
//TODO : Ajouter les attaques 
struct Action
{
	std::any information;
	std::function<bool(const ts::Entity& e, const ts::Scene& scene)> condition;
	std::function<void(ts::Entity& e, ts::Scene& scene, float dt, std::any& information)> execution;
};
struct AIComponent
{
	std::vector<Action> m_ActionLists;
};

void AIEnemiesSystem(ts::Scene& scene, float dt)
{
	scene.Query<AIComponent, TransformComponent>()
		.Where([&](ts::Entity e, const AIComponent& aiComponent, const TransformComponent& transform)
			{
				return scene.HasComponent<EnemyComponent>(e);
			})
		.Each([&](ts::Entity e, AIComponent& aiComponent, TransformComponent& transform)
			{
				for (auto& action : aiComponent.m_ActionLists)
				{
					if (action.condition(e, scene))
					{
						action.execution(e, scene, dt, action.information);
						break;
					}
				}
			});
}

struct PatrolData
{
	glm::vec3 center;
	float radius = 0.0f;
	glm::vec2 targetpos;

	float timer;

	std::mt19937 gen = std::mt19937{ std::random_device{}() };
	std::uniform_real_distribution<float> theta{ 0.0f, std::numbers::pi * 2.0f };
	std::uniform_real_distribution<float> r;
};

Action Patrol(glm::vec3& pos, float radius)
{
	float travelVelocity = 2.0f;
	Action patrol;
	patrol.information = PatrolData
	{
		.center = pos,
		.radius = radius * 2.0f,
		.targetpos = {1.0f,0.0f},
		.timer = 0.0f,
		.r = std::uniform_real_distribution<float>{0.0f, radius}
	};


	patrol.condition = [](const ts::Entity& e, const ts::Scene& scene) {return true; };
	patrol.execution = [travelVelocity](ts::Entity& e, ts::Scene& scene, float dt, std::any& information)
		{
			auto& data = std::any_cast<PatrolData&>(information);
			auto* transform = scene.GetComponent<TransformComponent>(e);

			data.timer += dt;
			if (data.timer >= 5.0f)
			{
				data.targetpos =
				{
					data.center.x + data.r(data.gen) * cosf(data.theta(data.gen)) - transform->GetPosition().x,
					data.center.z + data.r(data.gen) * sinf(data.theta(data.gen)) - transform->GetPosition().z
				};
				data.timer = 0.0f;
			}
			glm::vec3 targetPos = { data.targetpos.x, data.center.y,data.targetpos.y };

			if (glm::length(targetPos - transform->GetPosition()) > 0.1f)
				transform->SetPosition(transform->GetPosition() + glm::normalize(targetPos - transform->GetPosition()) * travelVelocity * dt);

		};

	return patrol;
}

//Action Attack()
//{
//
//}

#endif
