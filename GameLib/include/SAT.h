#pragma once
#include "AABB.h"
#include "OBB.h"
#include "Sphere.h"
#include "Collision.h"

namespace KGR
{
	class SeparatingAxisTheorem
	{
	public:
		static Collision3D CheckCollisionAABB3D(const AABB3D& box1, const AABB3D& box2);
		static Collision3D CheckCollisionOBB3D(const OBB3D& box1, const OBB3D& box2);
		static Collision3D CheckCollisionAABBvsOBB(const AABB3D& box1, const OBB3D& box2);
		static Collision3D CheckCollisionOBBvsSphere(const OBB3D& box, const Sphere& sphere);
		static Collision3D CheckCollisionAABBvsSphere(const AABB3D& box, const Sphere& sphere);
		static Collision3D CheckCollisionSpherevsSphere(const Sphere& sphere1, const Sphere& sphere2);
	};
}