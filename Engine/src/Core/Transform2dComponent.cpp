#include "Core/Transform2dComponent.h"

glm::vec2 TransformComponent2d::GetPosition() const
{
	return m_position.data;
}

void TransformComponent2d::SetPosition(const glm::vec2& other)
{
	m_position.data = other;
	m_position.isDirty = true;
}

void TransformComponent2d::Translate(const glm::vec2& other)
{
	m_position.data += other;
	m_position.isDirty = true;
}

glm::mat3 TransformComponent2d::GetTranslationMatrix()
{
	if (!m_position.isDirty)
			return m_positionMat;

		m_position.isDirty = false;

		m_positionMat =  glm::mat3(
			1, 0.0f, 0.0f,
			0.0f, 1, 0.0f,
			m_position.data.x, m_position.data.y, 1.0f);

	return m_positionMat;
}

glm::vec2 TransformComponent2d::GetScale() const
{
	return m_scale.data;
}

void TransformComponent2d::SetScale(const glm::vec2& other)
{
	m_scale.data = other;
	m_scale.isDirty = true;
}

glm::mat3 TransformComponent2d::GetScaleMatrix()
{

	if (!m_scale.isDirty)
		return m_scaleMat;

	m_scale.isDirty = false;
	m_scaleMat = glm::mat3(
		m_scale.data.x, 0.0f, 0.0f,
		0.0f, m_scale.data.y, 0.0f,
		0.0f, 0.0f, 1.0f);
	return m_scaleMat;
}

void TransformComponent2d::LookAt(const glm::vec2& target)
{
	glm::vec2 dir = target - m_position.data;
	float angle = std::atan2(dir.y, dir.x);
	SetRotation(angle);
}

void TransformComponent2d::LookAtDir(const glm::vec2& dir)
{
	float angle = std::atan2(dir.y, dir.x);
	SetRotation(angle);
}

float TransformComponent2d::GetRotation() const
{
	return m_angle.data;
}

void TransformComponent2d::SetRotation(float radAngle)
{
	m_angle.isDirty = true;
	m_angle.data = radAngle;
}

glm::mat3 TransformComponent2d::GetRotationMatrix()
{
	if (!m_angle.isDirty)
		return m_rotationMat;

	m_angle.isDirty = false;
	m_rotationMat = glm::mat3(
		std::cos(m_angle.data),  std::sin(m_angle.data), 0.0f,
		-std::sin(m_angle.data), std::cos(m_angle.data), 0.0f,
		0.0f, 0.0f, 1.0f);
	return m_rotationMat;
}

glm::mat3 TransformComponent2d::GetFullTransform()
{
	if (!IsDirty())
		return m_fullTransform;
	m_fullTransform = GetTranslationMatrix() * GetRotationMatrix()* GetScaleMatrix();
	return m_fullTransform;
} 

bool TransformComponent2d::IsDirty() const
{
	return m_position.isDirty || m_angle.isDirty || m_scale.isDirty;
}
