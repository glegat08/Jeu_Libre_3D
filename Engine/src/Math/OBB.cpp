#include "Math/OBB.h"
#include "glm/glm.hpp"

namespace KGR
{
    /**
     * @brief Constructs an oriented bounding box.
     *
     * The provided axis vectors are normalized to ensure the OBB maintains
     * a valid orthonormal basis. The half-size represents the extents along
     * each of these local axes.
     *
     * @param center   Center of the box in world space.
     * @param halfSize Half-size extents along each local axis.
     * @param xAxis    Local X axis (will be normalized).
     * @param yAxis    Local Y axis (will be normalized).
     * @param zAxis    Local Z axis (will be normalized).
     */
    OBB3D::OBB3D(const glm::vec3& center,
        const glm::vec3& halfSize,
        const glm::vec3& xAxis,
        const glm::vec3& yAxis,
        const glm::vec3& zAxis)
        : m_center(center), m_halfSize(halfSize)
    {
        m_axis[0] = glm::normalize(xAxis);
        m_axis[1] = glm::normalize(yAxis);
        m_axis[2] = glm::normalize(zAxis);
    }

    /**
     * @brief Returns the center of the OBB.
     */
    glm::vec3 OBB3D::GetCenter() const
    {
        return m_center;
    }

    /**
     * @brief Returns the half-size extents of the OBB.
     */
    glm::vec3 OBB3D::GetHalfSize() const
    {
        return m_halfSize;
    }

    /**
     * @brief Returns one of the three local axes of the OBB.
     *
     * @param index Axis index (0 = X, 1 = Y, 2 = Z).
     * @return Normalized axis vector, or (0,0,0) if index is invalid.
     */
    glm::vec3 OBB3D::GetAxis(int index) const
    {
        if (index < 0 || index > 2)
            return glm::vec3(0.0f); // Invalid index, return default

        return m_axis[index];
    }
}