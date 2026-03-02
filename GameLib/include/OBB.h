#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
    /**
     * @brief Oriented bounding box in 3D space.
     *
     * An OBB is defined by:
     * - a center position,
     * - a half‑size vector (extents along each local axis),
     * - three orthonormal axes representing the box orientation.
     *
     * This representation is commonly used for collision detection,
     * spatial queries, and physics simulations.
     */
    struct OBB3D
    {
        OBB3D() = default;

        /**
         * @brief Constructs an oriented bounding box.
         *
         * @param center     Center of the box in world space.
         * @param halfSize   Half‑size extents along each local axis.
         * @param xAxis      Local X axis (must be normalized).
         * @param yAxis      Local Y axis (must be normalized).
         * @param zAxis      Local Z axis (must be normalized).
         */
        OBB3D(const glm::vec3& center,
            const glm::vec3& halfSize,
            const glm::vec3& xAxis,
            const glm::vec3& yAxis,
            const glm::vec3& zAxis);

        /**
         * @brief Returns the center of the OBB.
         * @return Center position in world space.
         */
        glm::vec3 GetCenter() const;

        /**
         * @brief Returns the half‑size extents of the OBB.
         * @return Half‑size vector.
         */
        glm::vec3 GetHalfSize() const;

        /**
         * @brief Returns one of the three local axes of the OBB.
         *
         * @param index Axis index (0 = X, 1 = Y, 2 = Z).
         * @return Normalized axis vector.
         */
        glm::vec3 GetAxis(int index) const;

    private:
        glm::vec3 m_center;     ///< Center of the box.
        glm::vec3 m_halfSize;   ///< Half‑size extents along each axis.
        glm::vec3 m_axis[3];    ///< Local orthonormal axes (X, Y, Z).
    };
}