//#pragma once
//#include <glm/glm.hpp>
//
//#include <glm/ext/matrix_clip_space.hpp>
//
//
//struct CameraComponent
//{
//
//	float GetFov()const;
//	float GetAspect() const;
//	float GetNearRender()const;
//	float GetFarRender()const;
//	void SetFov(float fov);
//	void SetAspect(float aspect);
//	void SetNearRender(float nearR);
//	void SetFarRender(float farR);
//	bool IsActive() const;
//	void Enable();
//	void Disable();
//
//	void UpdateMatrix(const glm::vec4& modelMatrix);
//
//	glm::mat4 GetViewMatrix() const;
//	glm::mat4 GetProjectionMatrix() const;
//	static CameraComponent Create(float fov, float aspect, float near, float far, bool isActive);
//private:
//	float m_fov = 0,m_aspect = 0,m_nearR = 0, m_farR = 0;
//	bool m_isActive = false;
//	glm::mat4 m_viewMatrix = glm::mat4(1.0f);
//	glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
//};
