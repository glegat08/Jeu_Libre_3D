#include "Scene/Scene.h"

// ACTUALLY IN WORK -- NOT PART OF THE PROJECT

SceneEntity Scene::CreateEntity(const std::string& name, SceneEntity parent)
{
    SceneEntity e = m_registry.CreateEntity();

    m_registry.AddComponent<NameComponent>(e, NameComponent{ name });
    m_registry.AddComponent<TransformComponent>(e);
    m_registry.AddComponent<HierarchyComponent>(e, HierarchyComponent{ parent, {} });

    if (parent != NullEntity && m_registry.HasComponent<HierarchyComponent>(parent))
        m_registry.GetComponent<HierarchyComponent>(parent).m_children.push_back(e);

    return e;
}

SceneEntity Scene::CreateEmptyEntity()
{
    return m_registry.CreateEntity();
}

void Scene::DestroyEntity(SceneEntity e)
{
    if (m_registry.HasComponent<HierarchyComponent>(e))
    {
        auto& hierarchy = m_registry.GetComponent<HierarchyComponent>(e);

        if (hierarchy.m_parent != NullEntity
            && m_registry.HasComponent<HierarchyComponent>(hierarchy.m_parent))
        {
            auto& children = m_registry.GetComponent<HierarchyComponent>(hierarchy.m_parent).m_children;
            children.erase(std::remove(children.begin(), children.end(), e), children.end());
        }
    }

    DestroySubtree(e);
}

std::vector<SceneEntity> Scene::GetRootEntities()
{
    std::vector<SceneEntity> roots;

    for (SceneEntity e : m_registry.GetAllEntities())
    {
        if (!m_registry.HasComponent<HierarchyComponent>(e))
            continue;

        if (m_registry.GetComponent<HierarchyComponent>(e).m_parent == NullEntity)
            roots.push_back(e);
    }

    return roots;
}

SceneEntity Scene::FindByName(const std::string& name)
{
    for (SceneEntity e : m_registry.GetAllEntities())
    {
        if (!m_registry.HasComponent<NameComponent>(e))
            continue;

        if (m_registry.GetComponent<NameComponent>(e).name == name)
            return e;
    }

    return NullEntity;
}

SceneRegistry& Scene::GetRegistry()
{ 
    return m_registry;
}

const SceneRegistry& Scene::GetRegistry() const
{
    return m_registry; 
}

const std::vector<std::pair<std::string, InspectorFunction>>& Scene::GetInspectorRegistry() const
{
    return m_inspectorRegistry;
}

const std::vector<ComponentSerializerEntry>& Scene::GetSerializerRegistry() const
{
    return m_serializerRegistry;
}

std::unique_ptr<Scene> Scene::Clone()
{
    auto clone = std::make_unique<Scene>();

    clone->m_cloneRegistry = m_cloneRegistry;
    clone->m_inspectorRegistry = m_inspectorRegistry;
    clone->m_serializerRegistry = m_serializerRegistry;

    for (const SceneEntity e : m_registry.GetAllEntities())
    {
        const SceneEntity cloned = clone->m_registry.CreateEntity();
        for (const CloneFunction& fn : m_cloneRegistry)
            fn(e, cloned, m_registry, clone->m_registry);
    }

    return clone;
}

void Scene::DestroySubtree(SceneEntity e)
{
    if (m_registry.HasComponent<HierarchyComponent>(e))
    {
        // copy the list because DestroySubtree modifies it through child deletions
        auto children = m_registry.GetComponent<HierarchyComponent>(e).m_children;
        for (SceneEntity child : children)
            DestroySubtree(child);
    }

    m_registry.DestroyEntity(e);
}