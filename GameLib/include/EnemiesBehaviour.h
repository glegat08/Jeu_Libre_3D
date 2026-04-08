#ifndef ENEMIESBEHAVIOUR_H
#define ENEMIESBEHAVIOUR_H

#include <random>
#include <numbers>
#include <functional>
#include <any>
#include <cfloat>

#include "ts_ecs.h"
#include "Core/TrasformComponent.h"
#include "Core/Window.h"
#include "Components.h"



//TODO : COMMENTEZ 
//TODO : Changer le mouvement pour un slime
//TODO : Ajouter les attaques 
struct Action
{
	std::any information;
	std::function<bool(ts::Entity& e, ts::Scene& scene, std::any* information)> condition;
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
					if (action.condition(e, scene, &action.information))
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
		.radius = radius,
		.targetpos = {1.0f,0.0f},
		.timer = 0.0f,
		.r = std::uniform_real_distribution<float>{0.0f, radius}
	};


	patrol.condition = [](const ts::Entity& e, const ts::Scene& scene, std::any* information) {return true; };
	patrol.execution = [travelVelocity](ts::Entity& e, ts::Scene& scene, float dt, std::any& information)
		{
			auto& data = std::any_cast<PatrolData&>(information);
			auto* transform = scene.GetComponent<TransformComponent>(e);

			data.timer += dt;
			if (data.timer >= 5.0f)
			{
				data.targetpos =
				{
					transform->GetPosition().x + data.r(data.gen) * cosf(data.theta(data.gen)),
					transform->GetPosition().z + data.r(data.gen) * sinf(data.theta(data.gen))
				};
				data.timer = 0.0f;
			}
			glm::vec3 targetPos = { data.targetpos.x, data.center.y,data.targetpos.y };

			if (glm::length(targetPos - transform->GetPosition()) > 0.1f)
				transform->SetPosition(transform->GetPosition() + glm::normalize(targetPos - transform->GetPosition()) * travelVelocity * dt);

		};

	return patrol;
}

struct AttackData
{
	float r;
	int degat;
	glm::vec2 targetpos = {0.0f,0.0f};
	ts::Entity parcelle = ts::NullEntity;
	float timer;
};

Action Attack(const RadarComponent& radar)
{
	float travelVelocity = 4.0f;

	Action attack;
	attack.information = AttackData
	{
		.r = radar.r,
		.degat = 1,
		.timer = 0.0f
	};
	attack.condition = [&radar](ts::Entity& e, ts::Scene& scene, std::any* information)
		{
			auto& data = std::any_cast<AttackData&>(*information);
			data.parcelle = ts::NullEntity;
			glm::vec3 posEnemy = scene.GetComponent<TransformComponent>(e)->GetPosition();

			float distance_min = FLT_MAX;

			for (auto& parcelle : scene.Query<ParcelleComponent>().Collect())
			{
				glm::vec3 posParcelle = scene.GetComponent<TransformComponent>(parcelle)->GetPosition();
				glm::vec2 vector_Parcelle_Enemy = { posParcelle.x - posEnemy.x, posParcelle.z - posEnemy.z };
				auto length = glm::length(vector_Parcelle_Enemy);

				if (length < distance_min && length < data.r)
				{
					distance_min = length;
					data.targetpos = { posParcelle.x,posParcelle.z };
					data.parcelle = parcelle;
				}

			}

			return data.parcelle != ts::NullEntity;
		};
	attack.execution = [travelVelocity](ts::Entity& e, ts::Scene& scene, float dt, std::any& information)
		{
			auto& data = std::any_cast<AttackData&>(information);
			auto* transform = scene.GetComponent<TransformComponent>(e);
			auto* HealthParcelle = scene.GetComponent<HealtComponent>(data.parcelle);

			glm::vec2 vector_Parcelle_Enemy = { data.targetpos.x - transform->GetPosition().x, data.targetpos.y - transform->GetPosition().z };
			auto length = glm::length(vector_Parcelle_Enemy);

			data.timer += dt;

			if (length > 3.5f)
			{
				transform->SetPosition
			   (
				   transform->GetPosition() +
				   glm::vec3(
					   glm::normalize(vector_Parcelle_Enemy).x * travelVelocity * dt,
					   0.0f,
					   glm::normalize(vector_Parcelle_Enemy).y * travelVelocity * dt
				   )
			   );
				return;
			}
			
			if (data.timer >= 5.0f)
			{
				HealthParcelle->Health = HealthParcelle->Health - data.degat;
				data.timer = 0.0f;
			}
				
		};

	return attack;
}

#endif
