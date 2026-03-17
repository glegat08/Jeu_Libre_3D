#pragma once
#include <string>
#include <vector>
#include "ECS/Entities.h"

using SceneEntity = KGR::ECS::Entity::_64;

static constexpr SceneEntity NullEntity = static_cast<SceneEntity>(-1);

/** @brief display name of an entity */
struct NameComponent
{
    std::string name;
};

/** @brief parent/children links in the scene tree */
struct HierarchyComponent
{
    SceneEntity m_parent = NullEntity;
    std::vector<SceneEntity> m_children;
};