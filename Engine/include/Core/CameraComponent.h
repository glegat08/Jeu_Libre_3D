#pragma once
#include <glm/glm.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include "Tools/Util.h"


struct CameraComponent
{
	enum class Type
	{
		Ortho,
		Perspective
	};
	static CameraComponent Create(float fovY, float width, float height, float nearRender, float farRender, CameraComponent::Type type);

	void UpdateCamera(const glm::mat4& matrix);
	glm::mat4 GetView();
	glm::mat4 GetProj();
	void SetRendererInfo(float near, float far);
	void SetAspect(float width, float height);
	void SetType(CameraComponent::Type type);
	void SetFov(float fov);
	CameraComponent::Type GetType() const;
	float GetFov() const;
	float GetWidth() const;
	float GetHeight() const;
	float GetNearRender() const;
	float GetFarRender() const;
private:
	glm::mat4 m_fullTransform;
	DataDirty <glm::mat4> m_inverseData;
	DataDirty<glm::mat4> m_projData;

	float m_fovY = 0.0f, m_width = 0.0f, m_height = 0.0f, m_nearRender = 0.0f, m_farRender = 0.0f;
	CameraComponent::Type m_type = CameraComponent::Type::Perspective;
};


