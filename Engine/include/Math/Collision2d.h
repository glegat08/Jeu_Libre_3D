#pragma once
#include "AABB2D.h"


	struct CollisionComp2d
	{
		CollisionComp2d();
		void Update(const glm::vec2& pos, const glm::vec2& scale);
		KGR::AABB2D aabb;
	};
