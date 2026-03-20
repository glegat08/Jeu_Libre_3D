#include "Math/Sphere.h"

namespace KGR
{
    /**
     * @brief Constructs a sphere from its center and radius.
     */
    Sphere::Sphere(const glm::vec3& center, float radius)
        : m_center(center), m_radius(radius)
    {
    }

    /**
     * @brief Returns the center of the sphere.
     */
    glm::vec3 Sphere::GetCenter() const
    {
        return m_center;
    }

    /**
     * @brief Returns the radius of the sphere.
     */
    float Sphere::GetRadius() const
    {
        return m_radius;
    }
}