#include "Sphere.h"

namespace KGR
{
	Sphere::Sphere(const glm::vec3& center, float radius) : m_center(center), m_radius(radius)
	{
	}

	glm::vec3 Sphere::GetCenter() const
	{
		return m_center;
	}

	float Sphere::GetRadius() const
	{
		return m_radius;
	}
}