#include "Picker.h"
#include "CollisionComponent.h"
#include "Core/TrasformComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>

namespace KGR
{
    namespace Editor
    {
        Ray ScreenToWorld(glm::vec2 mousePos, glm::vec2 viewportPos,
            glm::vec2 viewportSize, CameraComponent& cam)
        {
            // pixel -> NDC [-1, 1], Y is flipped (ImGui is top-down, NDC is bottom-up)
            float ndcX = (2.0f * (mousePos.x - viewportPos.x) / viewportSize.x) - 1.0f;
            float ndcY = -(2.0f * (mousePos.y - viewportPos.y) / viewportSize.y) + 1.0f;

            // NDC -> view space (w=0 means direction, not position)
            glm::vec4 ray = glm::inverse(cam.GetProj()) * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            ray = { ray.x, ray.y, -1.0f, 0.0f };

            // view space -> world space
            glm::mat4 invView = glm::inverse(cam.GetView());
            return { glm::vec3(invView[3]), glm::normalize(glm::vec3(invView * ray)) };
        }

        float RayOBB(const Ray& ray, const OBB3D& obb)
        {
            glm::vec3 delta = ray.origin - obb.GetCenter();
            float tEnter = -std::numeric_limits<float>::max();
            float tExit = std::numeric_limits<float>::max();

            for (int i = 0; i < 3; ++i)
            {
                glm::vec3 axis = obb.GetAxis(i);
                float halfSize = obb.GetHalfSize()[i];
                float e = glm::dot(axis, delta);
                float f = glm::dot(axis, ray.dir);

                if (std::fabs(f) > 1e-6f)
                {
                    float t1 = (-e - halfSize) / f;
                    float t2 = (-e + halfSize) / f;
                    if (t1 > t2)
                        std::swap(t1, t2);

                    tEnter = std::max(tEnter, t1);
                    tExit = std::min(tExit, t2);
                    if (tEnter > tExit)
                        return -1.0f;
                }
                else if (e < -halfSize || e > halfSize)
                    return -1.0f;
            }

            return tEnter >= 0.0f ? tEnter : tExit;
        }

        float DefaultHitChecker::operator()(SceneEntity e, SceneRegistry& reg, const Ray& ray) const
        {
            auto& transform = reg.GetComponent<TransformComponent>(e);

            if (reg.HasComponent<CollisionComp>(e))
            {
                const CollisionComp& cc = reg.GetComponent<CollisionComp>(e);
                if (cc.collider)
                {
                    OBB3D obb = cc.collider->ComputeGlobalOBB(
                        transform.GetScale() * 2.0f,
                        transform.GetPosition(),
                        transform.GetOrientation());
                    return RayOBB(ray, obb);
                }
            }

            // fallback: sphere test around the entity origin
            glm::vec3 oc = ray.origin - transform.GetPosition();
            float b = glm::dot(oc, ray.dir);
            float c = glm::dot(oc, oc) - fallbackRadius * fallbackRadius;
            float disc = b * b - c;

            if (disc < 0.0f) 
                return -1.0f;

            float sq = std::sqrt(disc);
            float t0 = -b - sq;
            return (t0 >= 0.0f) ? t0 : (-b + sq);
        }

        void Picker::Select(SceneEntity e, bool addToExisting)
        {
            if (!addToExisting)
                m_selection.clear();

            if (addToExisting && m_selection.count(e))
            {
                m_selection.erase(e);
                // if we removed the primary, promote any remaining entity
                if (m_first == e)
                    m_first = m_selection.empty() ? NullEntity : *m_selection.begin();
            }
            else
            {
                m_selection.insert(e);
                m_first = e;
            }
        }

        void Picker::Deselect(SceneEntity e)
        {
            m_selection.erase(e);
            if (m_first == e)
                m_first = m_selection.empty() ? NullEntity : *m_selection.begin();
        }

        void Picker::ClearSelection()
        {
            m_selection.clear();
            m_first = NullEntity;
        }
    }
}