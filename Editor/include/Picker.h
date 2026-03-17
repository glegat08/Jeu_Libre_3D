#pragma once

#include <glm/glm.hpp>
#include <concepts>
#include <limits>
#include <unordered_set>
#include "Scene/Scene.h"
#include "Core/CameraComponent.h"
#include "OBB.h"

namespace KGR
{
    namespace Editor
    {
        /** @brief a ray in world space */
        struct Ray
        {
            glm::vec3 origin;
            glm::vec3 dir;
        };

        /** @brief mouse state for one picker frame */
        struct PickInput
        {
            Ray ray;
            bool isClicked; // left mouse button just pressed
            bool isHeld;    // Ctrl is held (multi-select)
        };

        /**
         * @brief concept for a callable that tests a ray against one entity
         *
         * signature : (SceneEntity, SceneRegistry&, const Ray&) -> float
         * return > 0 means hit at distance T, <= 0 means no hit
         */
        template<typename T>
        concept HitCheck = requires(T checker, SceneEntity e, SceneRegistry & reg, const Ray & ray)
        {
            { checker(e, reg, ray) } -> std::convertible_to<float>;
        };

        /**
         * @brief converts a screen pixel to a world-space ray
         * @param mousePos     cursor position in screen pixels
         * @param viewportPos  top-left corner of the viewport in screen pixels
         * @param viewportSize size of the viewport in pixels
         * @param cam          the editor camera
         * @return a ray starting at the camera position
         */
        Ray ScreenToWorld(glm::vec2 mousePos, glm::vec2 viewportPos,
            glm::vec2 viewportSize, CameraComponent& cam);

        /**
         * @brief tests a ray against an OBB using the slab method
         * @return distance T to the hit, or -1 if no hit
         */
        float RayOBB(const Ray& ray, const OBB3D& obb);

        /**
         * @brief iterates all entities, calls the hit checker, returns the closest hit
         * @param ray     the ray to test
         * @param scene   the scene to iterate
         * @param checker hit detection callable
         * @return the closest hit entity, or NullEntity
         */
        template<HitCheck Checker>
        SceneEntity PickEntity(const Ray& ray, Scene* scene, Checker&& checker)
        {
            if (!scene)
                return NullEntity;
            SceneRegistry& reg = scene->GetRegistry();
            SceneEntity best = NullEntity;
            float bestT = std::numeric_limits<float>::max();
            for (SceneEntity e : reg.GetAllEntities())
            {
                if (!reg.HasComponent<TransformComponent>(e))
                    continue;
                float t = checker(e, reg, ray);
                if (t > 0.0f && t < bestT)
                {
                    bestT = t;
                    best = e;
                }
            }
            return best;
        }

        /**
         * @brief default hit checker
         *
         * uses the entity's CollisionComp OBB if present,
         * otherwise falls back to a sphere around the entity origin
         */
        struct DefaultHitChecker
        {
            /** @param radius fallback sphere radius for entities without a collider */
            explicit DefaultHitChecker(float radius = 0.5f) : fallbackRadius(radius) {}
            float operator()(SceneEntity e, SceneRegistry& reg, const Ray& ray) const;
            float fallbackRadius;
        };

        /** @brief tracks which entities are hovered and selected each frame */
        class Picker
        {
        public:
            /**
             * @brief updates hovered and selected entities from the current mouse frame
             * @param input   mouse state built by the Viewport
             * @param scene   active scene to pick from
             * @param checker hit detection callable
             */
            template<HitCheck Checker>
            void Update(const PickInput& input, Scene* scene, Checker&& checker)
            {
                m_hovered = PickEntity(input.ray, scene, std::forward<Checker>(checker));
                if (!input.isClicked)
                    return;
                if (m_hovered == NullEntity)
                {
                    if (!input.isHeld)
                        ClearSelection();
                    return;
                }
                Select(m_hovered, input.isHeld);
            }

            /**
             * @brief adds or replaces the selection with entity e
             * @param e              the entity to select
             * @param addToExisting  true to add (Ctrl+click), false to replace
             */
            void Select(SceneEntity e, bool addToExisting);

            /** @brief removes entity e from the selection */
            void Deselect(SceneEntity e);

            /** @brief clears the entire selection */
            void ClearSelection();

            /** @return true if entity e is currently selected */
            bool isSelected(SceneEntity e) const { return m_selection.count(e) > 0; }

            /** @return all currently selected entities */
            const std::unordered_set<SceneEntity>& GetSelection() const { return m_selection; }

            /** @return the entity currently under the cursor, or NullEntity */
            SceneEntity GetHovered() const { return m_hovered; }

            /** @return the last entity that was explicitly clicked, or NullEntity */
            SceneEntity GetFirst() const { return m_first; }

        private:
            SceneEntity m_hovered = NullEntity;
            SceneEntity m_first = NullEntity;
            std::unordered_set<SceneEntity> m_selection;
        };
    }
}