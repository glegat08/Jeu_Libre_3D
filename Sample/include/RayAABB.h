#pragma once
#define NOMINMAX

#include "Core/Mesh.h"
#include "Core/TrasformComponent.h"

inline KGR::AABB3D ComputeWorldAABB(const Mesh& mesh, const TransformComponent& transform)
{
    glm::vec3 worldMin( 1e30f,  1e30f,  1e30f);
    glm::vec3 worldMax(-1e30f, -1e30f, -1e30f);

    for (uint32_t i = 0; i < mesh.GetSubMeshesCount(); ++i)
    {
        for (const Vertex& v : mesh.GetSubMesh(i).GetVertices())
        {
            glm::vec3 worldPos = transform.GetPosition() + v.pos * transform.GetScale();

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
