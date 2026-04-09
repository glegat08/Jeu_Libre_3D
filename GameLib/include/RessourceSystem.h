#ifndef RESSOURCE_SYSTEM_H
#define RESSOURCE_SYSTEM_H

#include <vector>
#include "ts_ecs.h"
#include "Core/TrasformComponent.h"
#include "Components.h"

struct RessourceComponent {};

void RessourceSpawnSystem(ts::Scene& scene)
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
                    scene.Add<RessourceComponent>(ressource, RessourceComponent{});

                    toDestroy.push_back(e);
                }
            });

    for (ts::Entity e : toDestroy)
        scene.Kill(e);
}

#endif