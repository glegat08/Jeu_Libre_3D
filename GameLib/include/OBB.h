#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
	struct OBB3D
	{
		OBB3D() = default;
		OBB3D(const glm::vec3& center, const glm::vec3& halfSize, const glm::vec3& xAxis, const glm::vec3& Yaxis, const glm::vec3& Zaxis);

		//GETTERS
		glm::vec3 GetCenter() const;
		glm::vec3 GetHalfSize() const;
		glm::vec3 GetAxis(int index) const;

	private:
		glm::vec3 m_center;
		glm::vec3 m_halfSize;
		//local x, y, z axes of the box (normalized)
		glm::vec3 m_axis[3];
	};
}