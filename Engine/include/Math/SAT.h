#pragma once
#include "Math/AABB.h"
#include "Math/OBB.h"
#include "Math/Sphere.h"
#include "Math/Collision.h"

namespace KGR
{
    /**
     * @brief Implements 3D collision tests using the Separating Axis Theorem (SAT).
     *
     * This class provides a collection of static functions for detecting collisions
     * between various primitive shapes:
     * - AABB vs AABB
     * - OBB vs OBB
     * - AABB vs OBB
     * - OBB vs Sphere
     * - AABB vs Sphere
     * - Sphere vs Sphere
     *
     * Each function returns a `Collision3D` object describing whether a collision
     * occurred, the penetration depth, and the collision normal.
     *
     * SAT is used for oriented boxes and mixed-shape tests, while simpler
     * analytical checks are used for spheres and AABBs.
     */
    class SeparatingAxisTheorem
    {
    public:
        /**
         * @brief Checks collision between two axis-aligned bounding boxes.
         *
         * @param box1 First AABB.
         * @param box2 Second AABB.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionAABB3D(const AABB3D& box1, const AABB3D& box2);

        /**
         * @brief Checks collision between two oriented bounding boxes using SAT.
         *
         * @param box1 First OBB.
         * @param box2 Second OBB.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionOBB3D(const OBB3D& box1, const OBB3D& box2);

        /**
         * @brief Checks collision between an AABB and an OBB using SAT.
         *
         * @param box1 AABB.
         * @param box2 OBB.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionAABBvsOBB(const AABB3D& box1, const OBB3D& box2);

        /**
         * @brief Checks collision between an OBB and a sphere.
         *
         * @param box OBB.
         * @param sphere Sphere.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionOBBvsSphere(const OBB3D& box, const Sphere& sphere);

        /**
         * @brief Checks collision between an AABB and a sphere.
         *
         * @param box AABB.
         * @param sphere Sphere.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionAABBvsSphere(const AABB3D& box, const Sphere& sphere);

        /**
         * @brief Checks collision between two spheres.
         *
         * @param sphere1 First sphere.
         * @param sphere2 Second sphere.
         * @return Collision3D Collision result.
         */
        static Collision3D CheckCollisionSpherevsSphere(const Sphere& sphere1, const Sphere& sphere2);
    };
}