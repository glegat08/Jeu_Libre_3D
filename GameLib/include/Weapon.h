#pragma once
#include <glm/glm.hpp>
#include "Core/Window.h"
#include "Core/Mesh.h"
#include "Core/Texture.h"
#include "Core/TrasformComponent.h"
#include "ECS/Registry.h"

struct LivingTimeComponent
{
	float timeToLive;
	void Update(float dt)
	{
		timeToLive -= dt;
	}
	bool IsExpired() const
	{
		return timeToLive <= 0.0f;
	}
};

namespace KGR
{
	namespace GameLib
	{
		enum class WeaponType
		{
			Shotgun,
			Auto,
			Sniper
		};

		struct WeaponData
		{
			float damage;
			float fireRate; //shots ratio
			float range;
			float spread;   //for shotgun
			int   maxAmmo;
			float reloadTime;
		};

		struct BulletComponent
		{
			glm::vec3 direction;
			float speed = 50.0f;
			float lifetime = 2.0f;
			float damage = 10.0f;
		};

		struct WeaponComponent
		{
			WeaponType current = WeaponType::Auto;
			WeaponData shotgun{ 50.0f, 1.0f, 15.0f, 0.5f, 8, 2.0f };
			WeaponData autoRifle{ 5.0f, 0.2f, 30.0f, 0.15f, 30, 1.5f };
			WeaponData sniper{ 200.0f, 1.5f, 100.0f, 0.0f, 5, 3.0f };

			float cooldown = 0.0f;
			int currentAmmo = 30;

			const WeaponData& GetCurrentWeaponData() const;
			void SwitchWeapon(WeaponType newType);
			template<typename Registry>
			void CreateBullet(Registry& registry, KGR::RenderWindow& window, const glm::vec3& position, const glm::vec3& direction);

			/*void Shoot();
			void Reload();*/
		};

		template<typename Registry>
		void WeaponComponent::CreateBullet(Registry& registry, KGR::RenderWindow& window, const glm::vec3& position, const glm::vec3& direction)
		{
			auto bullet = registry.CreateEntity();

			//Mesh
			MeshComponent meshComp;
			meshComp.mesh = &MeshLoader::Load("Models\\CUBE.obj", window.App());

			//Transform
			TransformComponent transform;
			transform.SetScale({ 0.1f, 0.1f, 0.1f });
			transform.SetPosition(position);
			transform.LookAtDir(direction);

			//Texture
			TextureComponent texture;
			texture.SetSize(meshComp.mesh->GetSubMeshesCount());
			for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
				texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));

			//Damage and direction
			BulletComponent bulletComp;
			bulletComp.direction = direction;
			bulletComp.damage = GetCurrentWeaponData().damage;
			bulletComp.speed = 50.0f;
			bulletComp.lifetime = 2.0f;

			LivingTimeComponent timeToLive;
			timeToLive.timeToLive = bulletComp.lifetime;

			registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, BulletComponent, LivingTimeComponent>
				(bullet, std::move(meshComp), std::move(transform), std::move(texture), std::move(bulletComp), std::move(timeToLive));
		}
	}
}