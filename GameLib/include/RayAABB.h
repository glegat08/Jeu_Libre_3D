#pragma once
#define NOMINMAX

#include <limits>
#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"

inline KGR::AABB3D ComputeWorldAABB(const Mesh& mesh, const TransformComponent& transform)
{
    glm::vec3 worldMin(1e30f, 1e30f, 1e30f);
    glm::vec3 worldMax(-1e30f, -1e30f, -1e30f);

    for (uint32_t i = 0; i < mesh.GetSubMeshesCount(); ++i)
    {
        for (const Vertex& v : mesh.GetSubMesh(i).GetVertices())
        {
            glm::vec3 worldPos = transform.GetPosition() + v.pos * (transform.GetScale() * 2.0f);

            worldMin.x = worldPos.x < worldMin.x ? worldPos.x : worldMin.x;
            worldMin.y = worldPos.y < worldMin.y ? worldPos.y : worldMin.y;
            worldMin.z = worldPos.z < worldMin.z ? worldPos.z : worldMin.z;

            worldMax.x = worldPos.x > worldMax.x ? worldPos.x : worldMax.x;
            worldMax.y = worldPos.y > worldMax.y ? worldPos.y : worldMax.y;
            worldMax.z = worldPos.z > worldMax.z ? worldPos.z : worldMax.z;
        }
    }

    return KGR::AABB3D(worldMin, worldMax);
}

inline bool RayAABB(const glm::vec3& origin, const glm::vec3& dir, const KGR::AABB3D& aabb, float maxDistance)
{
    glm::vec3 invDir = 1.0f / dir;

    glm::vec3 t1 = (aabb.GetMin() - origin) * invDir;
    glm::vec3 t2 = (aabb.GetMax() - origin) * invDir;

    float tMinX = t1.x < t2.x ? t1.x : t2.x;
    float tMinY = t1.y < t2.y ? t1.y : t2.y;
    float tMinZ = t1.z < t2.z ? t1.z : t2.z;

    float tMaxX = t1.x > t2.x ? t1.x : t2.x;
    float tMaxY = t1.y > t2.y ? t1.y : t2.y;
    float tMaxZ = t1.z > t2.z ? t1.z : t2.z;

    float tEnter = tMinX > tMinY ? tMinX : tMinY;
    tEnter = tEnter > tMinZ ? tEnter : tMinZ;

    float tExit = tMaxX < tMaxY ? tMaxX : tMaxY;
    tExit = tExit < tMaxZ ? tExit : tMaxZ;

    return tEnter <= tExit && tExit >= 0.0f && tEnter <= maxDistance;
}

/**
 * @brief tests a ray against a triangle using the Möller–Trumbore algorithm.
 * @param origin ray origin.
 * @param dir normalized ray direction.
 * @param v0 first triangle vertex in world space.
 * @param v1 second triangle vertex in world space.
 * @param v2 third triangle vertex in world space.
 * @param t distance from origin to the intersection point.
 * @return true if the ray hits the front face of the triangle.
 */
inline bool RayTriangle(const glm::vec3& origin, const glm::vec3& dir,
    const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
{
    const glm::vec3 e1 = v1 - v0;
    const glm::vec3 e2 = v2 - v0;
    const glm::vec3 h = glm::cross(dir, e2);
    const float det = glm::dot(e1, h);

    if (det > -1e-6f && det < 1e-6f)
        return false;

    const float invDet = 1.0f / det;
    const glm::vec3 s = origin - v0;
    const float u = invDet * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    const glm::vec3 q = glm::cross(s, e1);
    const float v = invDet * glm::dot(dir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = invDet * glm::dot(e2, q);
    return t > 1e-6f;
}

/**
 * @brief samples the world-space Y of a mesh surface directly below a given XZ position.
 * @param mesh terrain mesh to test against.
 * @param terrainPos world position of the terrain entity.
 * @param terrainScale scale of the terrain entity from GetScale().
 * @param x world-space X to sample.
 * @param z world-space Z to sample.
 * @param fallback returned when no surface is found below the sample point.
 * @return world-space Y of the nearest surface, or fallback.
 */
inline float SampleTerrainHeight(const Mesh& mesh, const glm::vec3& terrainPos,
    const glm::vec3& terrainScale, float x, float z, float fallback = 0.0f)
{
    static constexpr glm::vec3 kDown = { 0.0f, -1.0f, 0.0f };
    const glm::vec3 origin = { x, 10000.0f, z };

    float closestT = std::numeric_limits<float>::max();
    bool hit = false;

    for (uint32_t s = 0; s < mesh.GetSubMeshesCount(); ++s)
    {
        const auto& verts = mesh.GetSubMesh(s).GetVertices();
        const auto& indices = mesh.GetSubMesh(s).GetIndex();

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            const glm::vec3 v0 = terrainPos + verts[indices[i]].pos * terrainScale;
            const glm::vec3 v1 = terrainPos + verts[indices[i + 1]].pos * terrainScale;
            const glm::vec3 v2 = terrainPos + verts[indices[i + 2]].pos * terrainScale;

            // XZ early-out: skip triangles that can't be under this sample point.
            const float minX = std::min({ v0.x, v1.x, v2.x });
            const float maxX = std::max({ v0.x, v1.x, v2.x });
            const float minZ = std::min({ v0.z, v1.z, v2.z });
            const float maxZ = std::max({ v0.z, v1.z, v2.z });

            if (x < minX || x > maxX || z < minZ || z > maxZ)
                continue;

            float t;
            if (RayTriangle(origin, kDown, v0, v1, v2, t) && t < closestT)
            {
                closestT = t;
                hit = true;
            }
        }
    }

    return hit ? (origin.y - closestT) : fallback;
}