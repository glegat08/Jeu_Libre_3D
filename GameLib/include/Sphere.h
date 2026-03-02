#pragma once
#include "glm/vec3.hpp"

namespace KGR
{
    /**
     * @brief Simple 3D sphere primitive used for collision detection.
     *
     * A sphere is defined by:
     * - a center position in world space,
     * - a radius.
     *
     * This primitive is commonly used in broad‑phase collision detection,
     * distance checks, and SAT hybrid tests.
     */
    struct Sphere
    {
        Sphere() = default;

        /**
         * @brief Constructs a sphere from its center and radius.
         *
         * @param center Center of the sphere in world coordinates.
         * @param radius Radius of the sphere.
         */
        Sphere(const glm::vec3& center, float radius);

        /**
         * @brief Returns the center of the sphere.
         *
         * @return glm::vec3 Center position.
         */
        glm::vec3 GetCenter() const;

        /**
         * @brief Returns the radius of the sphere.
         *
         * @return float Radius value.
         */
        float GetRadius() const;

    private:
        glm::vec3 m_center; ///< Center of the sphere.
        float m_radius;     ///< Radius of the sphere.
    };
}