#pragma once
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "ECS/Registry.h"
#include "Core/TrasformComponent.h"
#include "Scene/SceneComponents.h"
#include <nlohmann/json.hpp>

// ACTUALLY IN WORK -- NOT PART OF THE PROJECT

using SceneRegistry = KGR::ECS::Registry<SceneEntity, 100>;
using CloneFunction = std::function<void(SceneEntity source, SceneEntity destination, SceneRegistry& from, SceneRegistry& to)>;
using InspectorFunction = std::function<void(SceneEntity e, SceneRegistry& registry)>;

// live handle -> stable integer stored in the save file
using HandleToIndex = std::unordered_map<SceneEntity, int>;

// stable integer from the save file -> live handle
using IndexToHandle = std::unordered_map<int, SceneEntity>;

/**
 * @brief writes one component's data into the entity's JSON object
 * @param e       the entity to read from
 * @param reg     the registry owning the entity
 * @param out     JSON object to write into
 * @param toIndex maps live handles to their file indices
 */
using WriteComponentFn = std::function<void(SceneEntity e, const SceneRegistry& reg,
    nlohmann::json& out,
    const HandleToIndex& toIndex)>;

/**
 * @brief reads one component's data from JSON and adds it to the entity
 * @param e        the entity to add the component to
 * @param reg      the registry owning the entity
 * @param in       JSON object to read from
 * @param toHandle maps file indices back to live handles
 */
using ReadComponentFn = std::function<void(SceneEntity e, SceneRegistry& reg,
    const nlohmann::json& in,
    const IndexToHandle& toHandle)>;

/*
* @brief write and read lambdas for one component type
*/
struct ComponentSerializerEntry
{
    WriteComponentFn write;
    ReadComponentFn read;
};

class Scene
{
public:
    /**
     * @brief creates a new entity with NameComponent, TransformComponent and HierarchyComponent
     * @param name   display name visible in the Hierarchy panel
     * @param parent optional parent entity, defaults to NullEntity (root level)
     * @return the new entity handle
     */
    SceneEntity CreateEntity(const std::string& name, SceneEntity parent = NullEntity);

    /**
     * @brief creates a raw entity with no components
     * @return the new entity handle
     */
    SceneEntity CreateEmptyEntity();

    /**
     * @brief returns the first entity whose NameComponent matches, or NullEntity
     * @param name the name to search for
     */
    SceneEntity FindByName(const std::string& name);

    /**
     * @brief destroys an entity and its entire subtree recursively
     * @param e the entity to destroy
     */
    void DestroyEntity(SceneEntity e);

    /**
     * @brief returns root-level entities (no parent)
     * @return a vector of root entity handles
     */
    std::vector<SceneEntity> GetRootEntities();

    /*
    * @return all registered inspector draw functions
    */
    const std::vector<std::pair<std::string, InspectorFunction>>& GetInspectorRegistry() const;

    /*
    * @return all registered serializer entries
    */
    const std::vector<ComponentSerializerEntry>& GetSerializerRegistry() const;

    /**
     * @brief creates a deep copy of the scene for Play mode
     * @return a unique_ptr to the cloned scene
     */
    std::unique_ptr<Scene> Clone();

    /**
     * @brief marks component T as cloneable
     *
     * usage : scene.RegisterClone<TransformComponent>();
     */
    template<typename T>
    void RegisterClone();

    /**
     * @brief registers an ImGui draw function for component T
     * @param label        header label shown in the Inspector panel
     * @param drawFunction the ImGui draw function to call
     *
     * usage :
     *   scene.RegisterInspector<MyComp>("My Component",
     *       [](SceneEntity e, SceneRegistry& reg) { ... });
     */
    template<typename T>
    void RegisterInspector(const std::string& label, InspectorFunction drawFunction);

    /**
     * @brief registers a write + read pair for component T
     * @param write lambda that fills a JSON object from the component
     * @param read  lambda that parses JSON and adds the component to the entity
     *
     * usage :
     *   scene.RegisterSerializer<NameComponent>(
     *       [](SceneEntity e, const SceneRegistry& reg, nlohmann::json& out, const HandleToIndex&)
     *       {
     *           if (!reg.HasComponent<NameComponent>(e)) return;
     *           out["NameComponent"]["name"] = reg.GetComponent<NameComponent>(e).name;
     *       },
     *       [](SceneEntity e, SceneRegistry& reg, const nlohmann::json& in, const IndexToHandle&)
     *       {
     *           if (!in.contains("NameComponent")) return;
     *           reg.AddComponent<NameComponent>(e, NameComponent{ in["NameComponent"]["name"] });
     *       });
     */
    template<typename T>
    void RegisterSerializer(WriteComponentFn write, ReadComponentFn read);
    template<typename T, typename... Args>
    void AddComponent(const SceneEntity e, Args&&... args);
    template<typename T>
    T& GetComponent(const SceneEntity e);
    template<typename T>
    bool HasComponent(const SceneEntity e) const;
    SceneRegistry& GetRegistry();
    const SceneRegistry& GetRegistry() const;

private:
    void DestroySubtree(SceneEntity e);
    SceneRegistry m_registry;
    std::vector<CloneFunction> m_cloneRegistry;
    std::vector<std::pair<std::string, InspectorFunction>> m_inspectorRegistry;
    std::vector<ComponentSerializerEntry> m_serializerRegistry;
};

template<typename T, typename... Args>
void Scene::AddComponent(const SceneEntity e, Args&&... args)
{
    m_registry.AddComponent<T>(e, std::forward<Args>(args)...);
}

template<typename T>
T& Scene::GetComponent(const SceneEntity e)
{
    return m_registry.GetComponent<T>(e);
}

template<typename T>
bool Scene::HasComponent(const SceneEntity e) const
{
    return m_registry.HasComponent<T>(e);
}

template<typename T>
void Scene::RegisterClone()
{
    m_cloneRegistry.push_back([](const SceneEntity source, const SceneEntity destination,
        SceneRegistry& from, SceneRegistry& to)
        {
            if (!from.HasComponent<T>(source))
                return;
            T copy = from.GetComponent<T>(source);
            to.AddComponent<T>(destination, std::move(copy));
        });
}

template<typename T>
void Scene::RegisterInspector(const std::string& label, InspectorFunction drawFunction)
{
    m_inspectorRegistry.push_back({ label, std::move(drawFunction) });
}

template<typename T>
void Scene::RegisterSerializer(WriteComponentFn write, ReadComponentFn read)
{
    m_serializerRegistry.push_back({ std::move(write), std::move(read) });
}