#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Tools/Util.h"

/**
 * @brief Component representing a camera in the scene.
 *
 * Stores:
 * - projection parameters (FOV, aspect ratio, near/far planes)
 * - projection matrix (dirty-flag optimized)
 * - inverse view matrix (dirty-flag optimized)
 * - camera type (orthographic or perspective)
 *
 * The view matrix is provided externally via UpdateCamera(), typically from
 * a TransformComponent.
 */
struct CameraComponent
{
    /**
     * @brief Supported projection types.
     */
    enum class Type
    {
        Ortho,
        Perspective
    };

    /**
     * @brief Factory method to create a fully configured camera.
     *
     * @param fovY Vertical field of view (radians).
     * @param width Viewport width.
     * @param height Viewport height.
     * @param nearRender Near clipping plane.
     * @param farRender Far clipping plane.
     * @param type Projection type.
     */
    static CameraComponent Create(float fovY,
        float width,
        float height,
        float nearRender,
        float farRender,
        CameraComponent::Type type);

    CameraComponent() {}

    /**
     * @brief Updates the camera's view matrix.
     *
     * @param matrix Full world transform of the camera (from TransformComponent).
     *
     * The view matrix is computed as the inverse of this transform.
     */
    void UpdateCamera(const glm::mat4& matrix);

    /**
     * @brief Returns the view matrix.
     *
     * Computed as inverse(fullTransform). Cached using dirty flags.
     */
    glm::mat4 GetView();

    /**
     * @brief Returns the projection matrix.
     *
     * Recomputed only when projection parameters change.
     */
    glm::mat4 GetProj();

    /**
     * @brief Sets near and far clipping planes.
     */
    void SetRendererInfo(float near, float far);

    /**
     * @brief Sets the aspect ratio using width and height.
     */
    void SetAspect(float width, float height);

    /**
     * @brief Sets the projection type (orthographic or perspective).
     */
    void SetType(CameraComponent::Type type);

    /**
     * @brief Sets the vertical field of view (radians).
     */
    void SetFov(float fov);

    /// @brief Returns the projection type.
    CameraComponent::Type GetType() const;

    /// @brief Returns the vertical field of view.
    float GetFov() const;

    /// @brief Returns the viewport width.
    float GetWidth() const;

    /// @brief Returns the viewport height.
    float GetHeight() const;

    /// @brief Returns the near clipping plane.
    float GetNearRender() const;

    /// @brief Returns the far clipping plane.
    float GetFarRender() const;

private:
    glm::mat4 m_fullTransform;            ///< World transform of the camera.
    DataDirty<glm::mat4> m_inverseData;   ///< Cached inverse transform (view matrix).
    DataDirty<glm::mat4> m_projData;      ///< Cached projection matrix.

    float m_fovY = 0.0f; ///< Vertical field of view (perspective).
    float m_width = 0.0f; ///< Viewport width.
    float m_height = 0.0f; ///< Viewport height.
    float m_nearRender = 0.0f; ///< Near clipping plane.
    float m_farRender = 0.0f; ///< Far clipping plane.

    CameraComponent::Type m_type = CameraComponent::Type::Perspective; ///< Projection type.
};