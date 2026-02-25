#include "Core/CameraComponent.h"
#include <stdexcept>
CameraComponent CameraComponent::Create(float fovY, float width, float height, float nearRender, float farRender,CameraComponent::Type type)
{
	CameraComponent camera;
	camera.SetFov(fovY);
	camera.SetAspect(width, height);
	camera.SetRendererInfo(nearRender, farRender);
	camera.SetType(type);
	camera.UpdateCamera(glm::mat4(1.0f));
	return camera;
}

void CameraComponent::UpdateCamera(const glm::mat4& matrix)
{
	if (matrix == m_fullTransform)
		return;
	m_fullTransform = matrix;
	m_inverseData.isDirty = true;
}

glm::mat4 CameraComponent::GetView()
{
	if (!m_inverseData.isDirty)
		return m_inverseData.data;

	m_inverseData.data = glm::inverse(m_fullTransform);
	m_inverseData.isDirty = false;
	return m_inverseData.data;
}

glm::mat4 CameraComponent::GetProj()
{
	if (!m_projData.isDirty)
		return m_projData.data;
	if (m_type == Type::Ortho)
		m_projData.data = glm::ortho(0.0f, m_width, 0.0f, m_height, m_nearRender, m_farRender);
	else
		m_projData.data = glm::perspective(m_fovY, m_width / m_height, m_nearRender, m_farRender);
	m_projData.isDirty = false;
	return m_projData.data;
}

void CameraComponent::SetRendererInfo(float near, float far)
{
	if (near > far)
		throw std::out_of_range("far must be bigger than near");
	m_nearRender = near;
	m_farRender = far;
	m_projData.isDirty = true;
}

void CameraComponent::SetAspect(float width, float height)
{
	m_width = width;
	m_height = height;
	m_projData.isDirty = true;
}

void CameraComponent::SetType(CameraComponent::Type type)
{
	m_type = type;
	m_projData.isDirty = true;
}


void CameraComponent::SetFov(float fov)
{
	m_fovY = fov;
	m_projData.isDirty = true;
}

CameraComponent::Type CameraComponent::GetType() const
{
	return m_type;
}

float CameraComponent::GetFov() const
{
	return m_fovY;
}

float CameraComponent::GetWidth() const
{
	return m_width;
}

float CameraComponent::GetHeight() const
{
	return m_height;
}

float CameraComponent::GetNearRender() const
{
	return m_nearRender;
}

float CameraComponent::GetFarRender() const
{
	return m_farRender;
}
