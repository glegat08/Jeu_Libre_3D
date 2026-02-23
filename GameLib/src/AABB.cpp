#include "AABB.h"

namespace KGR
{
	//Constructor that initializes the AABB with the given minimum and maximum points
	AABB3D::AABB3D(const glm::vec3& min, const glm::vec3& max)
		: m_min(min), m_max(max)
	{
	}

	//GETTERS

	glm::vec3 AABB3D::GetMin() const
	{
		return m_min;
	}

	glm::vec3 AABB3D::GetMax() const
	{
		return m_max;
	}

	glm::vec3 AABB3D::GetCenter() const
	{
		return (m_min + m_max) * 0.5f;
	}

	glm::vec3 AABB3D::GetSize() const
	{
		return m_max - m_min;
	}

	glm::vec3 AABB3D::GetHalfSize() const
	{
		return GetSize() * 0.5f;
	}
}