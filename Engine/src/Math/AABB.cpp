#include "Math/AABB.h"

namespace KGR
{
    /**
     * @brief Constructs an AABB from its minimum and maximum corners.
     *
     * @param min Lower corner of the bounding box.
     * @param max Upper corner of the bounding box.
     */
    AABB3D::AABB3D(const glm::vec3& min, const glm::vec3& max)
        : m_min(min), m_max(max)
    {
    }

    // --- GETTERS ---

    glm::vec3 AABB3D::GetMin() const
    {
        return m_min;
    }

    glm::vec3 AABB3D::GetMax() const
    {
        return m_max;
    }

    /**
     * @brief Computes the center point of the AABB.
     *
     * @return Center position (average of min and max).
     */
    glm::vec3 AABB3D::GetCenter() const
    {
        return (m_min + m_max) * 0.5f;
    }

    /**
     * @brief Computes the full size of the AABB along each axis.
     *
     * @return Size vector (max - min).
     */
    glm::vec3 AABB3D::GetSize() const
    {
        return m_max - m_min;
    }

    /**
     * @brief Computes half the size of the AABB along each axis.
     *
     * @return Half-size vector.
     */
    glm::vec3 AABB3D::GetHalfSize() const
    {
        return GetSize() * 0.5f;
    }
}