#include "Game.h"
#include "Tools/Chrono.h"
#include <array>
#include <random>

glm::vec3 GetPlayerPosition(ecsType& registry)
{
    auto players = registry.GetAllComponentsView<PlayerTag, TransformComponent>();
    for (auto& e : players)
        return registry.GetComponent<TransformComponent>(e).GetPosition();

    return glm::vec3(0.0f);
}

void LaserHitEnemies(ts::Scene& scene, const glm::vec3& origin, const glm::vec3& dir, float maxDist)
{
    std::vector<ts::Entity> toKill;

    scene.Query<EnemyComponent, HealtComponent, MeshComponent, TransformComponent>()
        .Each([&](ts::Entity e, EnemyComponent&, HealtComponent& hp, MeshComponent& mc, TransformComponent& tc)
            {
                if (!RayAABB(origin, dir, ComputeWorldAABB(*mc.mesh, tc), maxDist))
                    return;

                hp.Health -= 10;
                if (hp.Health <= 0)
                    toKill.push_back(e);
            });

    for (ts::Entity dead : toKill)
        scene.Kill(dead);
}

void RenderEnemies(ts::Scene& scene, KGR::RenderWindow* window, float dt)
{
    scene.Query<MeshComponent, TransformComponent, MaterialComponent>()
        .Each([&](ts::Entity e, MeshComponent& mc, TransformComponent& tc, MaterialComponent& mat)
            {
                int boneOffset = -1;

                if (scene.HasComponent<KGR::Animation::AnimationComponent>(e))
                {
                    auto& anim = scene.RequireComponent<KGR::Animation::AnimationComponent>(e);
                    anim.Update(dt);
                    boneOffset = window->App()->RegisterBoneMatrices(anim.GetLastBoneMatrices());
                }

                window->App()->RegisterRender(*mc.mesh, tc.GetFullTransform(),
                    mat.GetAllMaterials(), boneOffset);
            });
}

void RenderKGREntities(ecsType& registry, KGR::RenderWindow* window, float dt)
{
    auto es = registry.GetAllComponentsView<MeshComponent, TransformComponent, MaterialComponent>();
    for (auto& e : es)
    {
        int boneOffset = -1;
        if (registry.HasComponent<KGR::Animation::AnimationComponent>(e))
        {
            auto& anim = registry.GetComponent<KGR::Animation::AnimationComponent>(e);
            anim.Update(dt);
            boneOffset = window->App()->RegisterBoneMatrices(anim.GetLastBoneMatrices());
        }

        auto& mesh = registry.GetComponent<MeshComponent>(e);
        auto& transform = registry.GetComponent<TransformComponent>(e);
        auto& material = registry.GetComponent<MaterialComponent>(e);

        window->App()->RegisterRender(*mesh.mesh, transform.GetFullTransform(), 
            material.GetAllMaterials(), boneOffset);
    }
}

void RunGame(std::unique_ptr<KGR::RenderWindow>& window)
{
    ecsType registry{};
    ts::Scene scene;

    KGR::GLB::GLBCache glbCache;
    glbCache.Init(window->App());
    KGR::GLB::GLBNeutralTextures neutrals = glbCache.GetNeutrals();

    // ── Camera ────────────────────────────────────────────────────────────
    {
        auto cam = CameraComponent::Create(glm::radians(60.0f), window->GetSize().x, window->GetSize().y, 0.05f, 1500.0f, CameraComponent::Type::Perspective);

        TransformComponent transform;
        transform.SetPosition({ 0.0f, 0.0f, 0.0f });

        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(cam), std::move(transform));
    }

    // ── Player ────────────────────────────────────────────────────────────
    {
        TransformComponent transform;
        transform.SetPosition({ 0.0f, 0.0f, 0.0f });

        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(transform), PlayerTag{});
    }

    // ── Map ───────────────────────────────────────────────────────────────
    {
        const KGR::GLB::GLBAsset* mapAsset = glbCache.Get("Models/MapTest.glb", window->App());
        if (mapAsset)
            KGR::GLB::CreateGLBEntitiesFromNodes(registry, *mapAsset, glm::vec3{ 0.0f, -10.0f, 0.0f }, neutrals);
    }

    // ── Light ─────────────────────────────────────────────────────────────
    {
        LightComponent<LightData::Type::Directional> lc =
            LightComponent<LightData::Type::Directional>::Create({ 0.28f, 0.26f, 0.22f }, { 0.15f, 0.14f, 0.12f }, 1.0f);

        TransformComponent transform;
        transform.SetPosition({ 0.0f, 50.0f, 0.0f });
        transform.LookAtDir({ -0.3f, -1.0f, -0.3f });

        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(lc), std::move(transform));
    }

    // ── Laser ─────────────────────────────────────────────────────────────
    LaserState laserState;
    FPSCameraState cameraState;
    LaserInit(laserState, registry, window.get(), {});

    // ── Canon ─────────────────────────────────────────────────
    CanonState canonState;
    const KGR::GLB::GLBAsset* canonAsset = glbCache.Get("Models/Canon.glb", window->App());
    if (canonAsset)
    {
        Texture& canonSkin = TextureLoader::Load("Textures/Canon.png", window->App());
        KGR::GLB::GLBSkinOverride canonOverride{ .baseColor = &canonSkin };
        CanonInit(canonState, registry, *canonAsset, neutrals, &canonOverride);
    }

    // ── Mob asset ─────────────────────────────────────────────────────────
    const KGR::GLB::GLBAsset* mobAsset = glbCache.Get("Models/Mobs.glb", window->App());

    Texture& skinMob1 = TextureLoader::Load("Textures/Mob1.png", window->App());
    Texture& skinMob2 = TextureLoader::Load("Textures/Mob2.png", window->App());
    Texture& skinMob3 = TextureLoader::Load("Textures/Mob3.png", window->App());

    std::array<KGR::GLB::GLBSkinOverride, 3> mobSkins =
    {
        KGR::GLB::GLBSkinOverride{.baseColor = &skinMob1 },
        KGR::GLB::GLBSkinOverride{.baseColor = &skinMob2 },
        KGR::GLB::GLBSkinOverride{.baseColor = &skinMob3 }
    };

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> skinDist(0, static_cast<int>(mobSkins.size()) - 1);

    SpawnZone spawnZone{ .center = { 0.0f, 0.0f, 0.0f }, .radius = 100.0f };
    if (mobAsset)
        for (int i = 0; i < 5; ++i)
        {
            const auto& randomSkin = mobSkins[skinDist(rng)];
            SpawnEnemies(scene, *mobAsset, neutrals, spawnZone, &randomSkin);
        }

    float spawnTimer = 0.0f;
    constexpr float kSpawnInterval = 3.0f;

    KGR::Tools::Chrono<float> chrono;

    while (!window->ShouldClose())
    {
        float dt = chrono.GetDeltaTime();

        KGR::RenderWindow::PollEvent();
        window->Update();

        glm::vec3 front{ 0.0f, 0.0f, -1.0f };
        FPSCameraUpdate<ecsType, PlayerTag>(cameraState, registry, window.get(), dt, front);

        glm::vec3 playerPos = GetPlayerPosition(registry);

        CanonUpdate(canonState, registry, playerPos, front);

        spawnTimer += dt;
        if (spawnTimer >= kSpawnInterval && mobAsset)
        {
            spawnTimer = 0.0f;
            spawnZone.center = playerPos;
            const auto& randomSkin = mobSkins[skinDist(rng)];
            SpawnEnemies(scene, *mobAsset, neutrals, spawnZone, &randomSkin);
        }

        AIEnemiesSystem(window, scene, dt);

        LaserUpdate(laserState, registry, window.get(), playerPos, front);
        if (window->GetInputManager()->IsMousePressed(KGR::Mouse::Button1))
            LaserHitEnemies(scene, playerPos, front, laserState.maxDistance);

        //UpdateLightComponents<LightData::Type::Directional>(window, scene);

        {
            auto e = registry.GetAllComponentsView<LightComponent<LightData::Type::Directional>, TransformComponent>();
            for (auto es : e)
                window->RegisterLight(registry.GetComponent<LightComponent<LightData::Type::Directional>>(es), registry.GetComponent<TransformComponent>(es));
        }

        RenderKGREntities(registry, window.get(), dt);
        RenderEnemies(scene, window.get(), dt);

        window->Render({ 0.53f, 0.81f, 0.92f, 1.0f });
    }
}