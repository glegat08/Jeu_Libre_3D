#pragma once
#include "glm/vec2.hpp"
#include <vector>
#include <stdexcept>

namespace KGR
{
	struct AABB2D
	{
		AABB2D(const glm::vec2& min, const glm::vec2& max);
		AABB2D();
		glm::vec2 GetSize() const;

		glm::vec2 GetCenter() const;

		static AABB2D FromPoints(const std::vector<glm::vec2>& points);

		void Extend(const glm::vec2& point);


		bool operator==(const AABB2D& other) const;

		bool IsColliding(const glm::vec2& point) const;

		bool IsColliding(const AABB2D& other) const;
		glm::vec2 Amin;
		glm::vec2 Amax;
	};
}