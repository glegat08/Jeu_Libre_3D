#include "Collision.h"

namespace KGR
{
	//Constructor that initializes the collision result with the given values
	Collision3D::Collision3D(bool collide, float penetration, const glm::vec3& collisionNormal)
		: m_collide(collide), m_penetration(penetration), m_collisionNormal(collisionNormal)
	{
	}

	//GETTERS

	bool Collision3D::IsColliding() const
	{
		return m_collide;
	}

	float Collision3D::GetPenetration() const
	{
		return m_penetration;
	}

	glm::vec3 Collision3D::GetCollisionNormal() const
	{
		return m_collisionNormal;
	}
}