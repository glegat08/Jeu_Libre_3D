#include <filesystem>
#include <iostream>

#include "CollisionComponent.h"
#include "Core/CameraComponent.h"
#include "VulkanCore.h"
#include "_GLFW.h"
#include "Core/ManagerImple.h"
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"
#include "Core/LightComponent.h"
#include "Core/Spline.h"
#include "Core/Frenet.h"
#include "Core/Texture.h"
#include "Core/Window.h"
#include "ECS/Registry.h"
#include "ECS/Entities.h"
#include "Tools/Random.h"
#include "DebugRenderer.h"
#include "Event.h"
#include "EventBus.h"

#include "GameFiles.h"

// to move 
struct ControllerComponent {};

struct LivingComponent
{
	float health;
	bool isAlive;
};

struct TakeDamageComponent
{
	float damage;
};

struct WaveManager
{
	int currentWave = 0;
	int enemiesAlive = 0;

	bool isWaveActive = false;
	bool platformPaused = false;

	std::vector<float> wavePositions = { 0.2f, 0.5f, 0.8f };
};

int main(int argc, char** argv)
{
	std::filesystem::path exePath = argv[0];
	std::filesystem::path projectRoot = exePath.parent_path().parent_path().parent_path().parent_path().parent_path();
	KGR::RenderWindow::Init();
	KGR::RenderWindow window{ {1000,1000},"test",projectRoot / "Ressources" };
	window.GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);
	using ecsType = KGR::ECS::Registry<KGR::ECS::Entity::_64, 100>;
	auto registry = ecsType{};
	WaveManager waveManager;
	//// entity
	//{
	//	auto mesh = registry.CreateEntity();
	//	MeshComponent meshComp;
	//	meshComp.mesh = &MeshLoader::Load("Models\\briet_claire_decorsfantasy_grpB.obj", window.App());
	//	TransformComponent transform;
	//	TextureComponent texture;
	//	texture.SetSize(meshComp.mesh->GetSubMeshesCount());
	//	for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
	//		texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));
	//	registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, ControllerComponent>(mesh, std::move(meshComp), std::move(transform), std::move(texture), std::move(ControllerComponent{}));
	//}

	//Player
	{
		auto player = registry.CreateEntity();

		//Mesh
		MeshComponent meshComp;
		meshComp.mesh = &MeshLoader::Load("Models\\CUBE.obj", window.App());

		//Camera
		CameraComponent camComp = CameraComponent::Create(45.0f, static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y), 0.01f, 1000.0f, CameraComponent::Type::Perspective);

		//Transform
		TransformComponent camTransform;
		camTransform.SetPosition({ 0,3,5 });

		LivingComponent playerLife;
		playerLife.health = 5.0f;
		playerLife.isAlive = true;

		//Texture
		TextureComponent texture;
		texture.SetSize(meshComp.mesh->GetSubMeshesCount());
		for (int i = 0; i < meshComp.mesh->GetSubMeshesCount(); ++i)
			texture.AddTexture(i, &TextureLoader::Load("Textures\\BaseTexture.png", window.App()));

		CollisionComp collider;
		collider.collider = &ColliderManager::Load("playerCollider",meshComp.mesh);

		/*registry.AddComponents<MeshComponent, TransformComponent, TextureComponent, ControllerComponent>(mesh, std::move(meshComp), std::move(transform), std::move(texture), std::move(ControllerComponent{}));*/

		registry.AddComponents<MeshComponent, CameraComponent, TransformComponent, TextureComponent, ControllerComponent,
			PlayerComponent, KGR::GameLib::WeaponComponent, LivingComponent, CollisionComp>
			(player, std::move(meshComp), std::move(camComp), std::move(camTransform), std::move(texture), ControllerComponent{},
				PlayerComponent{}, KGR::GameLib::WeaponComponent{}, std::move(playerLife), std::move(collider));
	}

	auto colorTransform = [](const glm::vec3& color)
		{
			glm::vec3 result;
			result.x = color.x * 1 / 255;
			result.y = color.y * 1 / 255;
			result.z = color.z * 1 / 255;
			return result;
		};
	{
		auto light = registry.CreateEntity();
		auto lComp = LightComponent<LightData::Type::Directional>::Create(colorTransform({ 154,36,69 }), { 1,1,1 }, 1.0f);
		TransformComponent lTransform;
		lTransform.LookAt({ 0,-1,0 });
		registry.AddComponents<LightComponent<LightData::Type::Directional>, TransformComponent>(light, std::move(lComp), std::move(lTransform));
	}
	std::vector<glm::vec3> points{
	{  0.0f, 0.0f,  6.0f },   // départ
	{  3.5f, 0.0f,  5.0f },
	{  6.0f, 0.0f,  2.5f },
	{  6.5f, 0.0f,  0.0f },
	{  6.0f, 0.0f, -2.5f },
	{  3.5f, 0.0f, -5.0f },
	{  0.0f, 0.0f, -6.0f },
	{ -3.5f, 0.0f, -5.0f },
	{ -6.0f, 0.0f, -2.5f },
	{ -6.5f, 0.0f,  0.0f },
	{ -6.0f, 0.0f,  2.5f },
	{ -3.5f, 0.0f,  5.0f },
	{  0.0f, 0.0f,  6.0f }    // retour au départ pour boucler
	};

	//std::vector<glm::vec3> points
	//{
	//	//Loop
	//	{ -3.0f, 0.0f,  4.0f},
	//	{  0.0f, 0.0f,  0.0f},
	//	{  3.0f, 0.0f,  4.0f},
	//	{  6.0f, 0.0f,  0.0f},
	//	{  3.0f, 0.0f, -4.0f},
	//	{  0.0f, 0.0f,  0.0f},
	//	{ -3.0f, 0.0f,  4.0f},
	//	{ -6.0f, 0.0f,  0.0f},
	//	{ -3.0f, 0.0f, -4.0f},
	//	{  0.0f, 0.0f,  0.0f},
	//	{  3.0f, 0.0f,  4.0f},
	//	{  6.0f, 0.0f,  0.0f},
	//	{  3.0f, 0.0f, -4.0f},
	//	{  0.0f, 0.0f,  0.0f},
	//	{ -3.0f, 0.0f, -4.0f},
	//	{ -6.0f, 0.0f,  0.0f},
	//	{ -3.0f, 0.0f,  4.0f},
	//	{  0.0f, 0.0f,  0.0f},
	//	{  3.0f, 0.0f,  4.0f},
	//};

	HermitCurve curve = HermitCurve::FromPoints(points, 0);

	const float rmfStep = 0.001f;
	const int rmfSampleCount = static_cast<int>(curve.MaxT() / rmfStep) + 1;

	std::vector<glm::vec3> rmfPoints;
	rmfPoints.reserve(rmfSampleCount);

	for (int i = 0; i < rmfSampleCount; ++i)
		rmfPoints.push_back(curve.Compute(i * rmfStep));

	auto rmfForwardDirs = KGR::RMF::EstimateForwardDirs(rmfPoints);
	auto rmfFrames = KGR::RMF::BuildFrames(rmfPoints, rmfForwardDirs);

	static float curvesTest = 0.0f;

	do
	{
		/// EVENT PAS TOUCHE
		KGR::RenderWindow::PollEvent();
		window.Update();
		
		//Update Events


		//Update PAS TOUCHE
		static auto lastTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		// Update

		//Update player movement
		{
			auto es = registry.GetAllComponentsView<ControllerComponent, TransformComponent, CameraComponent>();
			auto* inputData = window.GetInputManager();
			for (auto& e : es)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				glm::vec3 dir = { 0.0f,0.0f,0.0f };
				if (inputData->IsKeyDown(KGR::Key::Z))
					dir.z = -1;
				if (inputData->IsKeyDown(KGR::Key::S))
					dir.z = 1;
				if (inputData->IsKeyDown(KGR::Key::Q))
					dir.x = -1;
				if (inputData->IsKeyDown(KGR::Key::D))
					dir.x = 1;
				if (inputData->IsKeyDown(KGR::SpecialKey::Space))
					dir.y = 1;
				if (inputData->IsKeyDown(KGR::SpecialKey::Shift))
					dir.y = -1;
				auto delta = inputData->GetMouseDelta();
				transform.RotateEuler<RotData::Orientation::Pitch>(-glm::radians(delta.y * deltaTime * 200));
				transform.RotateEuler<RotData::Orientation::Yaw>(-glm::radians(delta.x * deltaTime * 200));
				transform.Translate(dir * deltaTime);
			}
		}
		
		//Update player shoot
		{
			auto view = registry.GetAllComponentsView<PlayerComponent, KGR::GameLib::WeaponComponent, TransformComponent>();
			auto* inputData = window.GetInputManager();
			for (auto& e : view)
			{
				auto& weapon = registry.GetComponent<KGR::GameLib::WeaponComponent>(e);
				auto& transform = registry.GetComponent<TransformComponent>(e);
				weapon.cooldown -= deltaTime;
				weapon.UpdateReload(deltaTime);
				const auto& weaponData = weapon.GetCurrentWeaponData();
				if (inputData->IsKeyDown(KGR::Key::Num1))
					weapon.SwitchWeapon(KGR::GameLib::WeaponType::Shotgun);

				if (inputData->IsKeyDown(KGR::Key::Num2))
					weapon.SwitchWeapon(KGR::GameLib::WeaponType::Auto);

				if (inputData->IsKeyDown(KGR::Key::Num3))
					weapon.SwitchWeapon(KGR::GameLib::WeaponType::Sniper);

				if (inputData->IsKeyDown(KGR::Key::R))
					if (!weapon.isReloading && weapon.currentAmmo < weaponData.maxAmmo)
						weapon.StartReload();

				if (weapon.isReloading)
					continue;
				
				if (inputData->IsMouseDown(KGR::Mouse::Left))
				{
					if (weapon.cooldown <= 0.0f && weapon.currentAmmo > 0)
					{
						weapon.cooldown = weaponData.fireRate;
						weapon.currentAmmo--;
						glm::vec3 forward = transform.GetLocalAxe<RotData::Dir::Forward>();
						if (weaponData.spread > 0.0f)
						{
							float sx = KGR::Tools::Random().getRandomNumber(-weaponData.spread, weaponData.spread);
							float sy = KGR::Tools::Random().getRandomNumber(-weaponData.spread, weaponData.spread);
							forward = glm::normalize(forward + transform.GetLocalAxe<RotData::Dir::Right>() * sx + transform.GetLocalAxe<RotData::Dir::Up>() * sy);
						}

						if (weapon.current == KGR::GameLib::WeaponType::Shotgun)
							for (int i = 0; i < weaponData.maxAmmo; ++i)
							{
								glm::vec3 spreadDir = forward;

								float sx = KGR::Tools::Random().getRandomNumber(-weaponData.spread, weaponData.spread);
								float sy = KGR::Tools::Random().getRandomNumber(-weaponData.spread, weaponData.spread);
								spreadDir = glm::normalize(spreadDir + transform.GetLocalAxe<RotData::Dir::Right>() * sx + transform.GetLocalAxe<RotData::Dir::Up>() * sy);

								weapon.CreateBullet(registry, window, transform.GetPosition(), spreadDir);
							}
						else
							weapon.CreateBullet(registry, window, transform.GetPosition(), forward);
					}
				}
			}
		}

		//Update bullet
		{
			auto view = registry.GetAllComponentsView<KGR::GameLib::BulletComponent, TransformComponent, LivingTimeComponent, CollisionComp>();
			for (auto& e : view)
			{
				auto& bullet          = registry.GetComponent<KGR::GameLib::BulletComponent>(e);
				auto& bulletTransform = registry.GetComponent<TransformComponent>(e);
				auto& bulletCol       = registry.GetComponent<CollisionComp>(e);

				auto bulletOBB = bulletCol.collider->ComputeGlobalOBB(
								 bulletTransform.GetScale(),
								 bulletTransform.GetPosition(),
								 bulletTransform.GetOrientation());

				glm::vec3 newPos = bulletTransform.GetPosition();
				newPos += bullet.direction * bullet.speed * deltaTime;
				bulletTransform.SetPosition(newPos);

				bullet.lifetime -= deltaTime;
				if (bullet.lifetime <= 0.0f)
				{
					registry.DestroyEntity(e);
					continue;
				}

				//collision with enemy
				auto enemies = registry.GetAllComponentsView<KGR::GameLib::EnemyComponent, TransformComponent, CollisionComp>();
				for (auto enemyEntity : enemies)
				{
					auto& enemy = registry.GetComponent<KGR::GameLib::EnemyComponent>(enemyEntity);
					auto& enemyTransform = registry.GetComponent<TransformComponent>(enemyEntity);
					float dist = glm::length(enemyTransform.GetPosition() - bulletTransform.GetPosition());
					auto& enemyCol = registry.GetComponent<CollisionComp>(enemyEntity);

					auto enemyOBB = enemyCol.collider->ComputeGlobalOBB(
						enemyTransform.GetScale(),
						enemyTransform.GetPosition(),
						enemyTransform.GetOrientation());

					auto collision = KGR::SeparatingAxisTheorem::CheckCollisionOBB3D(bulletOBB, enemyOBB);

					if (collision.IsColliding())
					{
						enemy.health -= bullet.damage;
						registry.DestroyEntity(e);
						break;
					}
				}
			}
		}

		// UpdateIa
		/*static float spawnTimer = 0.0f;
		spawnTimer -= deltaTime;*/

		//Player pos
		glm::vec3 playerPos{ 0.0f };
		{
			auto view = registry.GetAllComponentsView<PlayerComponent, TransformComponent>();
			for (auto e : view)
			{
				playerPos = registry.GetComponent<TransformComponent>(e).GetPosition();
				break;
			}
		}

		////Spawn enemy
		//if (spawnTimer <= 0.0f)
		//{
		//	spawnTimer = 5.0f;

		//	KGR::GameLib::AIComponent ai;
		//	ai.SpawnEnemy(registry, window, playerPos);
		//}


		//Wave manager
		if (!waveManager.isWaveActive && waveManager.currentWave < waveManager.wavePositions.size())
		{
			if (curvesTest >= waveManager.wavePositions[waveManager.currentWave])
			{
				waveManager.isWaveActive = true;
				waveManager.platformPaused = true;

				int enemyCount = 5 + waveManager.currentWave * 3;

				for (int i = 0; i < enemyCount; i++)
				{
					KGR::GameLib::AIComponent ai;
					ai.SpawnEnemy(registry, window, playerPos);
					waveManager.enemiesAlive++;
				}
			}
		}
		
		//Update enemy
		{

			auto enemies = registry.GetAllComponentsView<KGR::GameLib::AIComponent, TransformComponent, KGR::GameLib::EnemyComponent, CollisionComp>();
			auto player = registry.GetAllComponentsView<PlayerComponent, TransformComponent, CollisionComp>();
			for (auto& enemyEntity : enemies)
			{
				auto& enemyTransform = registry.GetComponent<TransformComponent>(enemyEntity);
				auto& enemyAI		 = registry.GetComponent<KGR::GameLib::AIComponent>(enemyEntity);
				auto& enemy			 = registry.GetComponent<KGR::GameLib::EnemyComponent>(enemyEntity);
				auto& enemyCol		 = registry.GetComponent<CollisionComp>(enemyEntity);

				auto playerEntity = *player.begin();

				auto& playerTransform = registry.GetComponent<TransformComponent>(playerEntity);
				auto& playerCol = registry.GetComponent<CollisionComp>(playerEntity);

				auto playerOBB = playerCol.collider->ComputeGlobalOBB(
					playerTransform.GetScale(),
					playerTransform.GetPosition(),
					playerTransform.GetOrientation());

				auto enemyOBB = enemyCol.collider->ComputeGlobalOBB(
					enemyTransform.GetScale(),
					enemyTransform.GetPosition(),
					enemyTransform.GetOrientation());

				enemyAI.UpdateTarget(playerPos);

				glm::vec3 newPos = enemyAI.Update(deltaTime, enemyTransform.GetPosition(), enemy.speed);
				enemyTransform.SetPosition(newPos);

				auto collision = KGR::SeparatingAxisTheorem::CheckCollisionOBB3D(enemyOBB, playerOBB);

				enemy.timeSinceLastAttack -= deltaTime;

				if (collision.IsColliding() && enemy.timeSinceLastAttack <= 0.0f)
				{
					enemy.timeSinceLastAttack = enemy.attackCooldown;

					auto& life = registry.GetComponent<LivingComponent>(playerEntity);
					life.health -= enemy.damage;

					if (life.health <= 0.0f)
					{
						life.isAlive = false;
						std::cout << "Player Dead\n";
					}
				}

				if (collision.IsColliding())
					enemyTransform.SetPosition(enemyTransform.GetPosition() + collision.GetCollisionNormal() * collision.GetPenetration());

				if (enemy.health <= 0.0f)
				{
					registry.DestroyEntity(enemyEntity);

					waveManager.enemiesAlive--;

					std::cout << "Enemy defeated!\n";

					if (waveManager.enemiesAlive <= 0)
					{
						waveManager.isWaveActive = false;
						waveManager.platformPaused = false;
						waveManager.currentWave++;

						std::cout << "Wave cleared!\n";
					}
				}
			}
		}

		bool gameOver = false;
		//Is player dead
		{
			auto view = registry.GetAllComponentsView<PlayerComponent, LivingComponent>();
			for (auto& e : view)
			{
				auto& life = registry.GetComponent<LivingComponent>(e);
				if (!life.isAlive)
				{
					std::cout << "Game Over\n";
					gameOver = true;
				}
			}
		}

		//crash
		if (gameOver)
		{
			window.Render({ 0.0f,0.0f,0.0f,1.0f });
			continue;
		}
		
		//Update curve follower
		{
			auto es = registry.GetAllComponentsView<ControllerComponent, TransformComponent, MeshComponent>();
			for (auto& e : es)
			{
				auto& transform = registry.GetComponent<TransformComponent>(e);
				transform.SetPosition(curve.Compute(curvesTest));
				int frameIndex = glm::clamp(static_cast<int>(curvesTest / rmfStep), 0, static_cast<int>(rmfFrames.size() - 1));
				/*transform.SetOrientation(glm::quatLookAt(rmfFrames[frameIndex].forward, rmfFrames[frameIndex].up));*/
			}
		}

		if (!waveManager.platformPaused)
			curvesTest += 0.001f;
		if (curvesTest > curve.MaxT())
			curvesTest = 0.0f;

		{
			auto es = registry.GetAllComponentsView<CameraComponent, TransformComponent>();
			if (es.Size() != 1)
				throw std::runtime_error("need one and one cam");
			for (auto& e : es)
			{
				//registry.GetComponent<TransformComponent>(e).SetPosition({ camX, camY, camZ });
				//registry.GetComponent<TransformComponent>(e).LookAt({ 0.0f, 0.0f, 0.0f });
				registry.GetComponent<CameraComponent>(e).UpdateCamera(registry.GetComponent<TransformComponent>(e).GetFullTransform());
				window.RegisterCam(registry.GetComponent<CameraComponent>(e), registry.GetComponent<TransformComponent>(e));
			}
		}

		//TODO:
		// - following the player with the camera
		// - doing events where enemies spawn at specific times or when the player enters specific areas
		// - seing the curve path with debug draw
		// - doing the speadshot of the shotgun with multiple bullets with a random spread
		// - Collision detection between bullets and enemies and ennemies and player
		// - doing a health system for the player and the enemies and player can die and freeze the game when he dies
		// - doing a UI for ammos and health with ImGui or with a custom UI system
		// - doing a main menu and a game over screen
		// - add a new enemy mesh 
		// - finih 

		// Render Mesh
		{
			auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, TextureComponent>();

			for (auto& e : es)
				window.RegisterRender(registry.GetComponent<MeshComponent>(e), registry.GetComponent<TransformComponent>(e), registry.GetComponent<TextureComponent>(e));
		}

		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Point>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Point>>(e), registry.GetComponent<TransformComponent>(e));
		}
		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Spot>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Spot>>(e), registry.GetComponent<TransformComponent>(e));
		}

		{
			auto es = registry.GetAllComponentsView<LightComponent<LightData::Type::Directional>, TransformComponent>();
			for (auto& e : es)
				window.RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Directional>>(e), registry.GetComponent<TransformComponent>(e));
		}
		window.Render({ 0.53f,0.81f,0.92f ,1.0f });
	} while (!window.ShouldClose());


	window.Destroy();
	KGR::RenderWindow::End();
}