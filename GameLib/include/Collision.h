#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
	class Collision3D
	{
	public:
		Collision3D() = default;
		Collision3D(bool collide, float penetration, const glm::vec3& collisionNormal);

		//GETTERS
		bool IsColliding() const;
		float GetPenetration() const;
		glm::vec3 GetCollisionNormal() const;

	private:
		bool m_collide = false;
		float m_penetration = 0.0f;
		glm::vec3 m_collisionNormal = glm::vec3(0.0f);
	};
}