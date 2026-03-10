#pragma once
#include <memory>
#include <string>
#include <functional>
#include "ECS/Registry.h"
#include "Core/TrasformComponent.h"
#include "Scene/SceneComponents.h"

// ACTUALLY IN WORK -- NOT PART OF THE PROJECT

using SceneRegistry		= KGR::ECS::Registry<SceneEntity, 100>;

using CloneFunction		= std::function<void(SceneEntity source, SceneEntity destination, SceneRegistry& from, SceneRegistry& to)>;
using InspectorFunction = std::function<void(SceneEntity e, SceneRegistry& registry)>;

class Scene
{
public:

	/**
	 * @brief Creates a new entity with NameComponent, TransformComponent,
	 *        and HierarchyComponent, and registers it in the parent children list.
	 * 
	 * @param name   Display name visible in the Hierarchy panel.
	 * @param parent Optional parent entity ; defaults to NullEntity (root level).
	 * @return The new entity handle.
	 */
	SceneEntity CreateEntity(const std::string& name, SceneEntity parent = NullEntity);

	/**
	 * @brief Returns the first entity whose NameComponent matches, or NullEntity.
	 */
	SceneEntity FindByName(const std::string& name);

	/**
	 * @brief Destroys an entity and its entire subtree recursively.
	 *        Automatically unlinks the entity from its parent children list.
	 */
	void DestroyEntity(SceneEntity e);

	/**
	 * @brief Returns root-level entities (no parent).
	 *        Used by the Hierarchy panel to build the tree top-down.
	 */
	std::vector<SceneEntity> GetRootEntities();

	/**
	 * @brief Returns all registered inspector entries.
	 *        The Inspector panel iterates over this to draw each component.
	 */
	const std::vector<std::pair<std::string, InspectorFunction>>& GetInspectorRegistry() const;

	/**
	 * @brief Creates a deep copy of the scene for Play mode.
	 *
	 *        The editor keeps the original intact. The runtime operates on
	 *        the clone. On Stop, the clone is destroyed and the editor scene
	 *        is untouched.
	 */
	std::unique_ptr<Scene> Clone();

	/**
	* @brief Marks component T as cloneable.
	*
	*        Usage (engine init) :
	*          scene.RegisterClone<TransformComponent>();
	*          scene.RegisterClone<MeshComponent>();
	*
	*        Usage (editor init) :
	*          scene.RegisterClone<NameComponent>();
	*          scene.RegisterClone<HierarchyComponent>();
	*/
	template<typename T>
	void RegisterClone();

	/**
	 * @brief Registers an ImGui draw function for component T.
	 *
	 *        Usage (engine side) :
	 *          scene.RegisterInspector<LivingComponent>("Living",
	 *              [](SceneEntity e, SceneRegistry& reg)
	 *              {
	 *                  auto& c = reg.GetComponent<LivingComponent>(e);
	 *                  ImGui::DragFloat("Health", &c.health);
	 *                  ImGui::Checkbox("Alive",   &c.isAlive);
	 *              });
	 */
	template<typename T>
	void RegisterInspector(const std::string& label, InspectorFunction drawFunction);

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
