#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
	struct AABB3D
	{
		AABB3D() = default;
		AABB3D(const glm::vec3& min, const glm::vec3& max);

		//GETTERS
		glm::vec3 GetMin() const;
		glm::vec3 GetMax() const;
		glm::vec3 GetCenter() const;
		glm::vec3 GetSize() const;
		glm::vec3 GetHalfSize() const;

	private:
		glm::vec3 m_min;
		glm::vec3 m_max;
	};
}