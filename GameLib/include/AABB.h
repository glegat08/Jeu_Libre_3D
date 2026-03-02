#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
    /**
     * @brief Axis-aligned bounding box in 3D space.
     *
     * Represents a box aligned with the world axes, defined by a minimum
     * and maximum corner. Provides common geometric queries such as size,
     * center, and half-size.
     */
    struct AABB3D
    {
        AABB3D() = default;

        /**
         * @brief Constructs an AABB from its minimum and maximum corners.
         *
         * @param min Lower corner of the bounding box.
         * @param max Upper corner of the bounding box.
         */
        AABB3D(const glm::vec3& min, const glm::vec3& max);

        /**
         * @brief Returns the minimum corner of the AABB.
         * @return glm::vec3 Minimum (x, y, z) coordinates.
         */
        glm::vec3 GetMin() const;

        /**
         * @brief Returns the maximum corner of the AABB.
         * @return glm::vec3 Maximum (x, y, z) coordinates.
         */
        glm::vec3 GetMax() const;

        /**
         * @brief Returns the center point of the AABB.
         * @return glm::vec3 Center position.
         */
        glm::vec3 GetCenter() const;

        /**
         * @brief Returns the full size of the AABB along each axis.
         * @return glm::vec3 Size vector (max - min).
         */
        glm::vec3 GetSize() const;

        /**
         * @brief Returns half the size of the AABB along each axis.
         * @return glm::vec3 Half-size vector.
         */
        glm::vec3 GetHalfSize() const;

    private:
        glm::vec3 m_min; ///< Minimum corner of the AABB.
        glm::vec3 m_max; ///< Maximum corner of the AABB.
    };
}