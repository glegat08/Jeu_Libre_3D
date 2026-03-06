#pragma once
#include <glm/glm.hpp>
#include "Core/Window.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"
#include "CollisionComponent.h"

namespace KGR
{
	namespace GameLib
	{
		struct EnemyComponent
		{
			float speed;
			float attackRange;
			float damage;
			float health;
			float attackCooldown;
			float timeSinceLastAttack;
		};

		struct AIComponent
		{

			void UpdateTarget(const glm::vec3& target);
			glm::vec3 Update(float dt, const  glm::vec3& pos, float speed);
			glm::vec3 RandomRange(const glm::vec3& center, float minR, float maxR);
			template<typename Registry>
			void SpawnEnemy(Registry& registry, KGR::RenderWindow& window, const glm::vec3& playerPos);

		private:
			glm::vec3 m_target;
		};

		template<typename Registry>
		void KGR::GameLib::AIComponent::SpawnEnemy(Registry& registry, KGR::RenderWindow& window, const glm::vec3& playerPos)
		{
			auto enemy = registry.CreateEntity();

			//Mesh
			MeshComponent meshComp;
			meshComp.mesh = &MeshLoader::Load("Models\\noe_renegade.obj", window.App());

			//Transform
			TransformComponent transform;
			transform.SetPosition(RandomRange(playerPos, 15.0f, 20.0f));
			transform.SetScale({ 1.0f,1.0f,1.0f });

			//Texture
			TextureComponent texture;
			texture.SetSize(meshComp.mesh->GetSubMeshesCount());
			for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
				texture.AddTexture(i, &TextureLoader::Load("Textures\\basecolor.png", window.App()));

			CollisionComp collider;
			collider.collider = &ColliderManager::Load("enemyCollider", meshComp.mesh);

			AIComponent ai;
			ai.UpdateTarget(playerPos);

			EnemyComponent enemyComp;
			enemyComp.speed = 0.5f;
			enemyComp.attackRange = 1.0f;
			enemyComp.damage = 1.0f;
			enemyComp.health = 1.0f;
			enemyComp.attackCooldown = 1.0f;
			enemyComp.timeSinceLastAttack = 0.0f;


			registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, AIComponent, EnemyComponent, CollisionComp>
				(enemy, std::move(meshComp), std::move(transform), std::move(texture), std::move(ai), std::move(enemyComp), std::move(collider));
		}
	}
}