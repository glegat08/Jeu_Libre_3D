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

void RenderEnemies(ts::Scene& scene, KGR::RenderWindow* window)
{
    scene.Query<MeshComponent, TransformComponent, MaterialComponent>()
        .Each([&](ts::Entity, MeshComponent& mc, TransformComponent& tc, MaterialComponent& mat)
            {
                window->App()->RegisterRender(*mc.mesh, tc.GetFullTransform(), mat.GetAllMaterials(), -1);
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

        window->App()->RegisterRender(
            *registry.GetComponent<MeshComponent>(e).mesh,
            registry.GetComponent<TransformComponent>(e).GetFullTransform(),
            registry.GetComponent<MaterialComponent>(e).GetAllMaterials(),
            boneOffset);
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
        auto cam = CameraComponent::Create(glm::radians(60.0f), window->GetSize().x, window->GetSize().y, 0.01f, 200.0f, CameraComponent::Type::Perspective);

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
        const KGR::GLB::GLBAsset* mapAsset = glbCache.Get("Models/Map.glb", window->App());
        if (mapAsset)
        {
            Texture& mapSkin = TextureLoader::Load("Textures/Map.png", window->App());
			KGR::GLB::GLBSkinOverride mapOverride{ .baseColor = &mapSkin };
            KGR::GLB::CreateGLBEntity(registry, *mapAsset, glm::vec3{ 0.0f, -10.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 1.f, 1.f, 1.f }, neutrals, &mapOverride);
        }
    }

    // ── Directional light ─────────────────────────────────────────────────
    {
        auto light = LightComponent<LightData::Type::Directional>::Create({ 100.0f, 100.0f, 100.0f }, { 0.0f, -1.0f, -0.5f }, 1.0f);

        TransformComponent transform;
        transform.SetPosition({ 0.0f, 50.0f, 0.0f });

        auto e = registry.CreateEntity();
        registry.AddComponents(e, std::move(light), std::move(transform));
    }

    // ── Laser ─────────────────────────────────────────────────────────────
    LaserState laserState;
    FPSCameraState cameraState;
    LaserInit(laserState, registry, window.get(), {});

    // ── Canon (viewmodel) ─────────────────────────────────────────────────
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

    SpawnZone spawnZone{ .center = { 0.0f, 0.0f, 0.0f }, .radius = 10.0f };
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

        // ── update canon position every frame ─────────────────────────────
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

        UpdateLightComponents<LightData::Type::Directional>(window, scene);

        RenderKGREntities(registry, window.get(), dt);
        RenderEnemies(scene, window.get());

        window->Render({ 0.1f, 0.1f, 0.15f, 1.0f });
    }
}