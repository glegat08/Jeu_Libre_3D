#ifndef DEATH_SPAWN_H
#define DEATH_SPAWN_H

#include <vector>
#include <glm/vec3.hpp>

#include "ts_ecs.h"
#include "Core/TrasformComponent.h"
#include "Components.h"

struct DeathSpawnComponent
{
    void DeathSpawnSystem(ts::Scene& scene, float dt)
    {
        std::vector<ts::Entity> toDestroy;

        scene.Query<EnemyComponent, HealtComponent, TransformComponent>()
            .Each([&](ts::Entity e, const EnemyComponent&, const HealtComponent& health, const TransformComponent& transform)
                {
                    if (health.Health <= 0)
                    {
                        ts::Entity ressource = scene.Spawn();
                        TransformComponent ressourceTransform;
                        ressourceTransform.SetPosition(transform.GetPosition());
                        scene.Add<TransformComponent>(ressource, std::move(ressourceTransform));
                        scene.Add<DeathSpawnComponent>(ressource, DeathSpawnComponent{});

                        toDestroy.push_back(e);
                    }
                });

        for (ts::Entity e : toDestroy)
            scene.Kill(e);
    }
};
#endif