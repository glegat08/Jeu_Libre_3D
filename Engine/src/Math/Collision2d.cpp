#include "Math/Collision2d.h"
CollisionComp2d::CollisionComp2d() = default;

void CollisionComp2d::Update(const glm::vec2& pos, const glm::vec2& scale)
{
	glm::vec2 halfSize = scale / 2.0f;

	glm::vec2 min = halfSize;
	glm::vec2 max = halfSize;

	aabb.Amin = pos - min;
	aabb.Amax = pos + max;
}
