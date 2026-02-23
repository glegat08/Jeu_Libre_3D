#include "OBB.h"
#include "glm/glm.hpp"

namespace KGR
{
	//Constructor that initializes the OBB with the given center, half size, and local axes
	OBB3D::OBB3D(const glm::vec3& center, const glm::vec3& halfSize, const glm::vec3& xAxis, const glm::vec3& Yaxis, const glm::vec3& Zaxis)
		: m_center(center), m_halfSize(halfSize)
	{
		m_axis[0] = glm::normalize(xAxis);
		m_axis[1] = glm::normalize(Yaxis);
		m_axis[2] = glm::normalize(Zaxis);
	}

	glm::vec3 OBB3D::GetCenter() const
	{
		return m_center;
	}

	glm::vec3 OBB3D::GetHalfSize() const
	{
		return m_halfSize;
	}

	glm::vec3 OBB3D::GetAxis(int index) const
	{
		if (index < 0 || index > 2)
			//invalid so retrun default value
			return glm::vec3(0.0f);
		return m_axis[index];
	}
}