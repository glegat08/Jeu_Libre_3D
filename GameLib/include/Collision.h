#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
    /**
     * @brief Represents the result of a 3D collision test.
     *
     * This class stores whether a collision occurred, the penetration depth,
     * and the collision normal. It is typically returned by intersection
     * functions such as AABB–AABB, sphere–sphere, or raycast tests.
     */
    class Collision3D
    {
    public:
        Collision3D() = default;

        /**
         * @brief Constructs a collision result.
         *
         * @param collide          Whether a collision occurred.
         * @param penetration      Penetration depth along the collision normal.
         * @param collisionNormal  Normal vector pointing from the collider
         *                         toward the object being collided with.
         */
        Collision3D(bool collide, float penetration, const glm::vec3& collisionNormal);

        /**
         * @brief Indicates whether a collision occurred.
         * @return true if colliding, false otherwise.
         */
        bool IsColliding() const;

        /**
         * @brief Returns the penetration depth.
         * @return Penetration distance (0 if no collision).
         */
        float GetPenetration() const;

        /**
         * @brief Returns the collision normal.
         * @return Normalized direction of the collision.
         */
        glm::vec3 GetCollisionNormal() const;

    private:
        bool m_collide = false;                 ///< True if a collision occurred.
        float m_penetration = 0.0f;             ///< Penetration depth.
        glm::vec3 m_collisionNormal{ 0.0f };    ///< Collision normal vector.
    };
}