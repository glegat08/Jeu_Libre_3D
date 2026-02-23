#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
    struct Sphere
    {
        Sphere() = default;
        Sphere(const glm::vec3& center, float radius);
        glm::vec3 GetCenter() const;
        float GetRadius() const;
    private:
        glm::vec3 m_center;
        float m_radius;
    };
}