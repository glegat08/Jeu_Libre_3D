#include "SAT.h"
#define NOMINMAX
#include <algorithm>
#include <glm/geometric.hpp>

namespace KGR
{
    // =========================================================================
    // SAT HELPER FUNCTIONS
    // =========================================================================
    namespace SatHelper
    {
        /**
         * @brief Absolute dot product between two vectors.
         *
         * Used to avoid negative projection lengths when computing
         * extents along a separating axis.
         */
        static float ABSdot(const glm::vec3& a, const glm::vec3& b)
        {
            return std::fabs(glm::dot(a, b));
        }

        /**
         * @brief Computes the projection radius of an OBB onto a given axis.
         *
         * The projection is the sum of the absolute dot products between the
         * axis and each OBB axis, scaled by the corresponding half-size.
         *
         * @param box  Oriented bounding box.
         * @param axis Axis to project onto (does not need to be normalized).
         */
        static float OBBprojection(const KGR::OBB3D& box, const glm::vec3& axis)
        {
            return ABSdot(box.GetAxis(0), axis) * box.GetHalfSize().x +
                ABSdot(box.GetAxis(1), axis) * box.GetHalfSize().y +
                ABSdot(box.GetAxis(2), axis) * box.GetHalfSize().z;
        }

        /**
         * @brief Performs a SAT axis test between two OBBs.
         *
         * Projects both boxes onto the axis and checks for overlap.
         * If they overlap, updates the minimum penetration and collision normal.
         *
         * @param box1            First OBB.
         * @param box2            Second OBB.
         * @param axis            Axis to test (will be normalized internally).
         * @param deltaCenter     Vector from box1 center to box2 center.
         * @param minPenetration  Smallest penetration depth found so far.
         * @param collisionNormal Normal corresponding to the smallest penetration.
         *
         * @return true if projections overlap, false if a separating axis is found.
         */
        static bool AxisTest(const OBB3D& box1, const OBB3D& box2,
            const glm::vec3& axis,
            const glm::vec3& deltaCenter,
            float& minPenetration,
            glm::vec3& collisionNormal)
        {
            glm::vec3 normalizedAxis = glm::normalize(axis);

            float projection1 = OBBprojection(box1, normalizedAxis);
            float projection2 = OBBprojection(box2, normalizedAxis);

            float dist = ABSdot(deltaCenter, normalizedAxis);
            float penetration = projection1 + projection2 - dist;

            if (penetration <= 0)
                return false;

            if (penetration < minPenetration)
            {
                minPenetration = penetration;
                collisionNormal = normalizedAxis;
            }

            return true;
        }

        /**
         * @brief Converts an AABB into an OBB aligned with world axes.
         *
         * Useful for reusing the OBB SAT implementation for mixed AABB/OBB tests.
         */
        static OBB3D ConvertAABBtoOBB(const AABB3D& box)
        {
            return OBB3D(
                box.GetCenter(),
                box.GetHalfSize(),
                glm::vec3(1, 0, 0),
                glm::vec3(0, 1, 0),
                glm::vec3(0, 0, 1)
            );
        }
    }

    // =========================================================================
    // AABB vs AABB
    // =========================================================================

    /**
     * @brief Checks collision between two AABBs using axis overlap tests.
     *
     * This is a simplified SAT case where the axes are the world X/Y/Z axes.
     */
    Collision3D SeparatingAxisTheorem::CheckCollisionAABB3D(const AABB3D& box1,
        const AABB3D& box2)
    {
        glm::vec3 box1Center = box1.GetCenter();
        glm::vec3 box2Center = box2.GetCenter();
        glm::vec3 deltaCenter = box2Center - box1Center;

        glm::vec3 half1 = box1.GetHalfSize();
        glm::vec3 half2 = box2.GetHalfSize();

        float overlapX = half1.x + half2.x - std::fabs(deltaCenter.x);
        if (overlapX <= 0) return Collision3D(false, 0, glm::vec3(0));

        float overlapY = half1.y + half2.y - std::fabs(deltaCenter.y);
        if (overlapY <= 0) return Collision3D(false, 0, glm::vec3(0));

        float overlapZ = half1.z + half2.z - std::fabs(deltaCenter.z);
        if (overlapZ <= 0) return Collision3D(false, 0, glm::vec3(0));

        float penetration = overlapX;
        glm::vec3 normal;

        if (overlapX < overlapY && overlapX < overlapZ)
            normal = (deltaCenter.x < 0) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
        else if (overlapY < overlapZ)
        {
            penetration = overlapY;
            normal = (deltaCenter.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
        }
        else
        {
            penetration = overlapZ;
            normal = (deltaCenter.z < 0) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
        }

        return Collision3D(true, penetration, normal);
    }

    // =========================================================================
    // OBB vs OBB
    // =========================================================================

    /**
     * @brief Checks collision between two OBBs using the full SAT.
     *
     * Tests:
     * - 3 axes of box1
     * - 3 axes of box2
     * - 9 cross-product axes
     *
     * If any axis separates the boxes, no collision occurs.
     */
    Collision3D SeparatingAxisTheorem::CheckCollisionOBB3D(const OBB3D& box1,
        const OBB3D& box2)
    {
        using namespace SatHelper;

        glm::vec3 deltaCenter = box2.GetCenter() - box1.GetCenter();

        float minPenetration = std::numeric_limits<float>::max();
        glm::vec3 collisionNormal;

        // Axes of box1
        for (int i = 0; i < 3; ++i)
            if (!AxisTest(box1, box2, box1.GetAxis(i), deltaCenter, minPenetration, collisionNormal))
                return Collision3D(false, 0, glm::vec3(0));

        // Axes of box2
        for (int i = 0; i < 3; ++i)
            if (!AxisTest(box1, box2, box2.GetAxis(i), deltaCenter, minPenetration, collisionNormal))
                return Collision3D(false, 0, glm::vec3(0));

        // Cross-product axes
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (!AxisTest(box1, box2,
                    glm::cross(box1.GetAxis(i), box2.GetAxis(j)),
                    deltaCenter, minPenetration, collisionNormal))
                    return Collision3D(false, 0, glm::vec3(0));

        // Ensure normal points from box1 to box2
        if (glm::dot(collisionNormal, deltaCenter) < 0)
            collisionNormal = -collisionNormal;

        return Collision3D(true, minPenetration, collisionNormal);
    }

    // =========================================================================
    // AABB vs OBB
    // =========================================================================

    /**
     * @brief Checks collision between an AABB and an OBB using SAT.
     *
     * The AABB is converted to an OBB aligned with world axes, then the
     * standard OBB vs OBB SAT test is applied.
     */
    Collision3D SeparatingAxisTheorem::CheckCollisionAABBvsOBB(const AABB3D& box1,
        const OBB3D& box2)
    {
        OBB3D box1AsOBB = SatHelper::ConvertAABBtoOBB(box1);
        return CheckCollisionOBB3D(box1AsOBB, box2);
    }



    // =========================================================================
    // OBB vs Sphere
    // =========================================================================

    /**
     * @brief Checks collision between an OBB and a Sphere.
     *
     * The sphere center is projected into the OBB's local coordinate system to
     * determine the closest point on the box. If the distance between this point
     * and the sphere center is less than the sphere radius, a collision occurs.
     */
	Collision3D SeparatingAxisTheorem::CheckCollisionOBBvsSphere(const OBB3D& box, const Sphere& sphere)
	{
		//Find the closest point on the OBB to the sphere center
		glm::vec3 deltaCenter = sphere.GetCenter() - box.GetCenter();

		//Project the delta onto the local axes of the box to get the local coordinates of the sphere center relative to the box
		float localX = glm::dot(deltaCenter, box.GetAxis(0));
		float localY = glm::dot(deltaCenter, box.GetAxis(1));
		float localZ = glm::dot(deltaCenter, box.GetAxis(2));

		//Clamp the local coordinates to the extents of the box to find the closest point on the box to the sphere center
		float clampedX = (std::max)(-box.GetHalfSize().x, (std::min)(localX, box.GetHalfSize().x));
		float clampedY = (std::max)(-box.GetHalfSize().y, (std::min)(localY, box.GetHalfSize().y));
		float clampedZ = (std::max)(-box.GetHalfSize().z, (std::min)(localZ, box.GetHalfSize().z));

		//Convert the closest point from local coordinates back to world coordinates
		glm::vec3 closest =
			box.GetCenter() +
			box.GetAxis(0) * clampedX +
			box.GetAxis(1) * clampedY +
			box.GetAxis(2) * clampedZ;

		//Calculate the distance from the closest point to the sphere center
		glm::vec3 delta = sphere.GetCenter() - closest;
		float distSq = glm::dot(delta, delta);

		//If the distance is greater than the sphere radius, there is no collision
		if (distSq > sphere.GetRadius() * sphere.GetRadius())
			return Collision3D(false, 0.0f, glm::vec3(0.0f));

		//Collision detected, calculate penetration depth and collision normal
		float dist = std::sqrt(distSq);
		float penetration = sphere.GetRadius() - dist;

		//Calculate the collision normal, making sure to handle the case where the sphere center is exactly on the closest point to avoid division by zero
		glm::vec3 normal =
			(dist > 0.0001f) ? delta / dist : glm::vec3(1, 0, 0);

		//Collision detected, return the result with penetration depth and collision normal
		return Collision3D(true, penetration, normal);
	}

    // =========================================================================
    // AABB vs Sphere
    // =========================================================================

    /**
    * @brief Checks collision between an AABB and a Sphere.
    *
    * The sphere center is clamped to the AABB extents to find the closest point
    * on the box. If the distance between this point and the sphere center is
    * less than the sphere radius, a collision occurs.
    */
	Collision3D SeparatingAxisTheorem::CheckCollisionAABBvsSphere(const AABB3D& box, const Sphere& sphere)
	{
		//Find the closest point on the AABB to the sphere center by clamping the sphere center to the extents of the box
		glm::vec3 min = box.GetCenter() - box.GetHalfSize();
		glm::vec3 max = box.GetCenter() + box.GetHalfSize();

		//Clamp the sphere center to the box extents to find the closest point on the box to the sphere center
		glm::vec3 closest;
		closest.x = (std::max)(min.x, (std::min)(sphere.GetCenter().x, max.x));
		closest.y = (std::max)(min.y, (std::min)(sphere.GetCenter().y, max.y));
		closest.z = (std::max)(min.z, (std::min)(sphere.GetCenter().z, max.z));

		//Calculate the distance from the closest point to the sphere center
		glm::vec3 delta = sphere.GetCenter() - closest;
		float distSq = glm::dot(delta, delta);

		//If the distance is greater than the sphere radius, there is no collision
		if (distSq > sphere.GetRadius() * sphere.GetRadius())
			return Collision3D(false, 0.0f, glm::vec3(0.0f));

		//Collision detected, calculate penetration depth and collision normal
		float dist = std::sqrt(distSq);
		float penetration = sphere.GetRadius() - dist;

		//Calculate the collision normal, making sure to handle the case where the sphere center is exactly on the closest point to avoid division by zero
		glm::vec3 normal =
			(dist > 0.0001f) ? delta / dist : glm::vec3(1, 0, 0);

		//Collision detected, return the result with penetration depth and collision normal
		return Collision3D(true, penetration, normal);
	}

    // =========================================================================
    // Sphere vs Sphere
    // =========================================================================
    
    /**
    * @brief Checks collision between two spheres.
    *
    * A collision occurs if the distance between the sphere centers is less than
    * the sum of their radii. Penetration depth and collision normal are derived
    * from the center separation.
    */
	Collision3D SeparatingAxisTheorem::CheckCollisionSpherevsSphere(const Sphere& sphere1, const Sphere& sphere2)
	{
		//Calculate the distance between the centers of the two spheres
		glm::vec3 deltaCenter = sphere1.GetCenter() - sphere2.GetCenter();
		float distanceSquared = glm::dot(deltaCenter, deltaCenter);
		float radiusSum = sphere1.GetRadius() + sphere2.GetRadius();

		//If the distance squared is greater than the sum of the radii squared, there is no collision
		if (distanceSquared >= radiusSum * radiusSum)
			return Collision3D(false, 0.0f, glm::vec3(0.0f));

		//Collision detected, calculate penetration depth and collision normal
		float dist = std::sqrt(distanceSquared);
		float penetration = radiusSum - dist;

		//Calculate the collision normal, making sure to handle the case where the sphere centers are exactly on top of each other to avoid division by zero
		glm::vec3 normal =
			(dist > 0.0001f) ? deltaCenter / dist : glm::vec3(1, 0, 0);

		//Collision detected, return the result with penetration depth and collision normal
		return Collision3D(true, penetration, normal);
	}
}