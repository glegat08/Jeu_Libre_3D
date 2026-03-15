#include "Core/TrasformComponent.h"

glm::vec3 TransformComponent::GetPosition() const
{
	return m_position.data;
}

void TransformComponent::SetPosition(const glm::vec3& other)
{
	m_position.data = other;
	m_position.isDirty = true;
}

void TransformComponent::Translate(const glm::vec3& other)
{
	m_position.data += other;
	m_position.isDirty = true;
}

glm::mat4 TransformComponent::GetTranslationMatrix()
{
	if (!m_position.isDirty)
		return m_positionMat;

	m_positionMat = glm::translate(glm::mat4(1.0f), m_position.data);
	m_position.isDirty = false;

	return m_positionMat;
}

glm::vec3 TransformComponent::GetScale() const
{
	return m_scale.data;
}

void TransformComponent::SetScale(const glm::vec3& other)
{
	m_scale.data = other / 2.0f;
	m_scale.isDirty = true;
}


glm::mat4 TransformComponent::GetScaleMatrix()
{
	if (!m_scale.isDirty)
		return m_scaleMat;

	m_scaleMat = glm::scale(glm::mat4(1.0f), m_scale.data);
	m_scale.isDirty = false;

	return m_scaleMat;
}

glm::quat TransformComponent::GetOrientation() const
{
	return m_rotation.data;
}

void TransformComponent::SetOrientation(const glm::quat& other)
{
	m_rotation.data = glm::normalize(other);
	UpdateEulerAngle();
	m_rotation.isDirty = true;
}



glm::vec3 TransformComponent::GetRotation() const
{
	return m_eulerAngle;
}

void TransformComponent::SetRotation(const glm::vec3& other)
{
	m_eulerAngle = other;
	UpdateQuaternion();
	m_rotation.isDirty = true;
}



glm::mat4 TransformComponent::GetRotationMatrix()
{
	if (!m_rotation.isDirty)
		return m_rotationMat;

	m_rotationMat = glm::toMat4(glm::normalize(m_rotation.data));
	m_rotation.isDirty = false;

	return m_rotationMat;
}

glm::mat4 TransformComponent::GetFullTransform()
{
	if (!IsDirty())
		return m_fullTransform;

	m_fullTransform = GetTranslationMatrix() * GetRotationMatrix() * GetScaleMatrix();	
	return m_fullTransform;
}

void TransformComponent::UpdateEulerAngle()
{
	m_eulerAngle = glm::eulerAngles(m_rotation.data);
}

void TransformComponent::UpdateQuaternion()
{
	m_rotation.data = glm::normalize(glm::quat(m_eulerAngle));
}

bool TransformComponent::IsDirty() const
{
	return m_position.isDirty || m_rotation.isDirty || m_scale.isDirty;
}
