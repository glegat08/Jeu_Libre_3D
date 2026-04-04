#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Tools/Util.h"
#include <concepts>

/**
 * @brief Default coordinate system representation.
 *
 * Provides canonical up/right/forward axes used by rotation utilities.
 */
struct Rep
{
    static constexpr glm::vec3 up = { 0, 1, 0 };
    static constexpr glm::vec3 right = { 1, 0, 0 };
    static constexpr glm::vec3 forward = { 0, 0, -1 };
};

/**
 * @brief Concept ensuring a type provides up/right/forward vectors.
 */
template<typename Type>
concept IsValidRep = requires
{
    { Type::up }      -> std::convertible_to<const glm::vec3&>;
    { Type::right }   -> std::convertible_to<const glm::vec3&>;
    { Type::forward } -> std::convertible_to<const glm::vec3&>;
};

/**
 * @brief Rotation utilities for directional and axis conversions.
 *
 * Provides:
 * - canonical direction vectors (Up, Down, Forward, etc.)
 * - canonical rotation axes (Yaw, Pitch, Roll)
 * - customizable coordinate systems through Rep types
 */
struct RotData
{
    /**
     * @brief Canonical directions relative to a representation.
     */
    enum class Dir
    {
        Up,
        Down,
        Forward,
        Backward,
        Left,
        Right
    };

    /**
     * @brief Converts a direction enum into a normalized vector.
     *
     * @tparam dir Direction to convert.
     * @tparam rep Coordinate system representation (default: Rep).
     */
    template<Dir dir, IsValidRep rep = Rep>
    static glm::vec3 ToVector()
    {
        if constexpr (dir == Dir::Up)
            return glm::normalize(rep::up);
        else if constexpr (dir == Dir::Down)
            return glm::normalize(-rep::up);
        else if constexpr (dir == Dir::Forward)
            return glm::normalize(rep::forward);
        else if constexpr (dir == Dir::Backward)
            return glm::normalize(-rep::forward);
        else if constexpr (dir == Dir::Right)
            return glm::normalize(rep::right);
        else
            return glm::normalize(-rep::right);
    }

    /**
     * @brief Canonical rotation axes.
     */
    enum class Orientation
    {
        Yaw,
        Pitch,
        Roll
    };

    /**
     * @brief Converts an orientation enum into a normalized axis vector.
     *
     * @tparam orientation Rotation axis.
     * @tparam rep Coordinate system representation (default: Rep).
     */
    template<Orientation orientation, IsValidRep rep = Rep>
    static glm::vec3 ToAxes()
    {
        if constexpr (orientation == Orientation::Pitch)
            return glm::normalize(rep::right);
        else if constexpr (orientation == Orientation::Yaw)
            return glm::normalize(rep::up);
        else
            return glm::normalize(rep::forward);
    }
};



/**
 * @brief Component storing position, rotation, and scale.
 *
 * Provides:
 * - quaternion and Euler rotation
 * - LookAt utilities
 * - local axis extraction
 * - full transform matrix generation
 * - dirty‑flag optimization for matrix recomputation
 */
class TransformComponent
{
public:

    // -------------------------------------------------------------------------
    // POSITION
    // -------------------------------------------------------------------------

    /// @brief Returns the world position.
    glm::vec3 GetPosition() const;

    /// @brief Sets the world position.
    void SetPosition(const glm::vec3& other);

    /// @brief Translates the position by a delta vector.
    void Translate(const glm::vec3& other);

    /// @brief Returns the translation matrix.
    glm::mat4 GetTranslationMatrix();


    // -------------------------------------------------------------------------
    // SCALE
    // -------------------------------------------------------------------------

    /// @brief Returns the scale vector.
    glm::vec3 GetScale() const;

    /// @brief Sets the scale vector.
    void SetScale(const glm::vec3& other);

    /// @brief Returns the scale matrix.
    glm::mat4 GetScaleMatrix();


    // -------------------------------------------------------------------------
    // ORIENTATION (QUATERNION)
    // -------------------------------------------------------------------------

    /// @brief Returns the orientation quaternion.
    glm::quat GetOrientation() const;

    /// @brief Sets the orientation quaternion.
    void SetOrientation(const glm::quat& other);

    /**
     * @brief Rotates the orientation using a quaternion rotation around a canonical axis.
     *
     * @tparam orientation Axis of rotation.
     * @tparam rep Coordinate system representation.
     * @param angleRad Rotation angle in radians.
     */
    template<RotData::Orientation orientation, IsValidRep rep = Rep>
    void RotateQuat(float angleRad);

    /**
     * @brief Rotates the transform to look at a world-space target.
     *
     * @tparam rep Coordinate system representation.
     */
    template<IsValidRep rep = Rep>
    void LookAt(const glm::vec3& target);

    /**
     * @brief Rotates the transform to look in a direction from origin.
     *
     * @tparam rep Coordinate system representation.
     */
    template<IsValidRep rep = Rep>
    void LookAtDir(const glm::vec3& target);


    // -------------------------------------------------------------------------
    // ROTATION (EULER)
    // -------------------------------------------------------------------------

    /// @brief Returns Euler angles (XYZ order).
    glm::vec3 GetRotation() const;

    /// @brief Sets Euler angles.
    void SetRotation(const glm::vec3& other);

    /**
     * @brief Rotates the transform using Euler angles around a canonical axis.
     *
     * @tparam orientation Axis of rotation.
     * @tparam rep Coordinate system representation.
     */
    template<RotData::Orientation orientation, IsValidRep rep = Rep>
    void RotateEuler(float angleRad);



    /// @brief Returns the rotation matrix.
    glm::mat4 GetRotationMatrix();


    // -------------------------------------------------------------------------
    // AXES
    // -------------------------------------------------------------------------

    /**
     * @brief Returns a local axis transformed by the current rotation.
     *
     * @tparam dir Canonical direction.
     * @tparam rep Coordinate system representation.
     */
    template<RotData::Dir dir, IsValidRep rep = Rep>
    glm::vec3 GetLocalAxe() const;


    // -------------------------------------------------------------------------
    // FULL TRANSFORM
    // -------------------------------------------------------------------------

    /// @brief Returns the full transform matrix (T * R * S).
    glm::mat4 GetFullTransform();

    /// @brief Returns true if any transform component has changed.
    bool IsDirty() const;

private:

    /// @brief Updates Euler angles from quaternion.
    void UpdateEulerAngle();

    /// @brief Updates quaternion from Euler angles.
    void UpdateQuaternion();

    DataDirty<glm::quat> m_rotation = { true, glm::quat{1,0,0,0} };
    glm::mat4 m_rotationMat = glm::mat4{ 1.0f };
    glm::vec3 m_eulerAngle = glm::vec3{ 0,0,0 };

    DataDirty<glm::vec3> m_position = { true, glm::vec3{0,0,0} };
    glm::mat4 m_positionMat = glm::mat4{ 1.0f };

    DataDirty<glm::vec3> m_scale = { true, glm::vec3{0.5f,0.5f,0.5f} };
    glm::mat4 m_scaleMat = glm::mat4{ 1.0f };

    glm::mat4 m_fullTransform = glm::mat4{ 1.0f };

    bool m_fullTransformDirty = true;
};


// -----------------------------------------------------------------------------
// TEMPLATE IMPLEMENTATIONS
// -----------------------------------------------------------------------------

template <RotData::Orientation orientation, IsValidRep rep>
void TransformComponent::RotateQuat(float angleRad)
{
    glm::quat deltaRotation = glm::angleAxis(angleRad, RotData::ToAxes<orientation, rep>());
    m_rotation.data = glm::normalize( m_rotation.data * deltaRotation);
    UpdateEulerAngle();
    m_rotation.isDirty = true;
}

template <IsValidRep rep>
void TransformComponent::LookAt(const glm::vec3& target)
{
    glm::vec3 forward = glm::normalize(target - m_position.data);
    auto up = RotData::ToVector<RotData::Dir::Up, rep>();
    if (abs(glm::dot(forward, up)) > 0.999f)
        up = RotData::ToVector<RotData::Dir::Forward, rep>();
    m_rotation.data = glm::quatLookAt(forward, up);
    UpdateEulerAngle();
    m_rotation.isDirty = true;
}

template <IsValidRep rep>
void TransformComponent::LookAtDir(const glm::vec3& target)
{
    glm::vec3 forward = glm::normalize(target - glm::vec3{ 0,0,0 });
    auto up = RotData::ToVector<RotData::Dir::Up, rep>();
    if (abs(glm::dot(forward, up)) > 0.999f)
        up = RotData::ToVector<RotData::Dir::Forward, rep>();
    m_rotation.data = glm::quatLookAt(forward, up);
    UpdateEulerAngle();
    m_rotation.isDirty = true;
}

template <RotData::Orientation orientation, IsValidRep rep>
void TransformComponent::RotateEuler(float angleRad)
{
    m_eulerAngle += angleRad * RotData::ToAxes<orientation, rep>();
    UpdateQuaternion();
    m_rotation.isDirty = true;
}

template <RotData::Dir dir, IsValidRep rep>
glm::vec3 TransformComponent::GetLocalAxe() const
{
    return m_rotation.data * RotData::ToVector<dir, rep>();
}