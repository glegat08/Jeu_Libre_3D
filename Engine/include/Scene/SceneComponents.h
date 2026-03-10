#pragma once
#include <string>
#include <vector>
#include "ECS/Entities.h"

using SceneEntity = KGR::ECS::Entity::_64;

static constexpr SceneEntity NullEntity = static_cast<SceneEntity>(-1);

struct NameComponent
{
	std::string name;
};

struct HierarchyComponent
{
	SceneEntity m_parent = NullEntity;
	std::vector<SceneEntity> m_children;
};