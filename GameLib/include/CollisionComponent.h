#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "AABB.h"
#include "OBB.h"
#include "Core/Mesh.h"
#include "Core/RessourcesManager.h"

/**
 * @brief Collider storing a local-space AABB and providing global AABB/OBB computation.
 *
 * This structure represents the collision volume of a mesh in local space.
 * It can compute:
 * - a global AABB (scaled + translated)
 * - a global OBB (scaled + rotated + translated)
 *
 * The local AABB is typically generated from mesh vertex data.
 */
struct Collider
{
    KGR::AABB3D localBox; ///< Local-space axis-aligned bounding box.

    /**
     * @brief Computes the global AABB using scale and position.
     *
     * @param scale Object scale.
     * @param pos World position.
     * @return Global-space AABB.
     */
    KGR::AABB3D ComputeGlobalAABB(const glm::vec3& scale, const glm::vec3& pos) const
    {
        return KGR::AABB3D(localBox.m_min * scale + pos,
            localBox.m_max * scale + pos);
    }

    /**
     * @brief Computes the global OBB using scale, position, and orientation.
     *
     * @param scale Object scale.
     * @param pos World position.
     * @param orientation World rotation (quaternion).
     * @return Global-space oriented bounding box.
     */
    KGR::OBB3D ComputeGlobalOBB(const glm::vec3& scale,
        const glm::vec3& pos,
        const glm::quat& orientation) const
    {
        glm::vec3 forward = orientation * glm::vec3{ 0, 0, -1 };
        glm::vec3 right = orientation * glm::vec3{ 1, 0, 0 };
        glm::vec3 up = orientation * glm::vec3{ 0, 1, 0 };

        glm::vec3 center = pos + orientation * (localBox.GetCenter() * scale);
        glm::vec3 halfSize = localBox.GetHalfSize() * scale;

        return KGR::OBB3D(center, halfSize, right, up, forward);
    }
};

/**
 * @brief Component wrapper storing a pointer to a collider.
 *
 * Used by ECS or entity systems to attach collision data to an entity.
 */
struct CollisionComp
{
    Collider* collider = nullptr;
};

/**
 * @brief Generates a collider from a mesh by computing its local AABB.
 *
 * Iterates through all vertices of all submeshes to compute the minimum
 * and maximum extents of the mesh.
 *
 * @param filePath Path to the mesh resource (unused here but required by ResourceManager).
 * @param mesh Pointer to the mesh.
 * @return A unique_ptr to a newly created Collider.
 */
inline std::unique_ptr<Collider> GenerateBox(const std::string& filePath, Mesh* mesh)
{
    std::unique_ptr<Collider> collide = std::make_unique<Collider>();

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (int i = 0; i < mesh->GetSubMeshesCount(); ++i)
    {
        auto& sub = mesh->GetSubMesh(i);
        auto vertices = sub.GetVertices();

        for (int j = 0; j < sub.VertexCount(); ++j)
        {
            minX = std::min(minX, vertices[j].pos.x);
            minY = std::min(minY, vertices[j].pos.y);
            minZ = std::min(minZ, vertices[j].pos.z);

            maxX = std::max(maxX, vertices[j].pos.x);
            maxY = std::max(maxY, vertices[j].pos.y);
            maxZ = std::max(maxZ, vertices[j].pos.z);
        }
    }

    KGR::AABB3D box;
    box.m_min = { minX, minY, minZ };
    box.m_max = { maxX, maxY, maxZ };

    collide->localBox = box;

    return collide;
}

/**
 * @brief Resource manager alias for colliders generated from meshes.
 *
 * Automatically generates a collider when loading a mesh resource.
 */
using ColliderManager =
KGR::ResourceManager<Collider, KGR::TypeWrapper<Mesh*>, GenerateBox>;
