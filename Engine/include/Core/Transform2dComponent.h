#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "Tools/Util.h"
#include <concepts>


class TransformComponent2d
{
public:
    glm::vec2 GetPosition() const;
    void SetPosition(const glm::vec2& other);
    void Translate(const glm::vec2& other);
    glm::mat3 GetTranslationMatrix();

    glm::vec2 GetScale() const;
    void SetScale(const glm::vec2& other);
    glm::mat3 GetScaleMatrix();

    void LookAt(const glm::vec2& target);
    void LookAtDir(const glm::vec2& dir);
    float GetRotation() const;
    void SetRotation(float radAngle);
    glm::mat3 GetRotationMatrix();

    glm::mat3 GetFullTransform();
    bool IsDirty() const;

private:

    DataDirty<float> m_angle {true,0.0f};
    glm::mat3 m_rotationMat = glm::mat3{ 1.0f };


    DataDirty<glm::vec2> m_position = { true, glm::vec2{0,0} };
    glm::mat3 m_positionMat = glm::mat3{ 1.0f };

    DataDirty<glm::vec2> m_scale = { true, glm::vec2{1,1} };
    glm::mat3 m_scaleMat = glm::mat3{ 1.0f };

    glm::mat3 m_fullTransform = glm::mat3{ 1.0f };
};

