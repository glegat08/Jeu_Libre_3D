#pragma once

#include <glm/glm.hpp>
#include "imgui.h"
#include "Core/Frenet.h"

class DebugDraw3D
{
public:
	void BeginFrame(const glm::mat4& view, const glm::mat4& proj, float screenW, float screenH);

	void DrawLine(const glm::vec3& a, const glm::vec3& b, ImU32 color, float thickness = 2.0f);
	void DrawPoint(const glm::vec3& pos, float radius, ImU32 color);
	void DrawFrame(const glm::vec3& pos, const KGR::CurveFrame& frame, float axisLength,
		float thickness = 2.5f, uint8_t alpha = 255);

private:
	ImVec2 WorldToScreen(const glm::vec3& pos) const;

	glm::mat4  m_vp       = glm::mat4(1.0f);
	float      m_screenW  = 0.0f;
	float      m_screenH  = 0.0f;
	ImDrawList* m_drawList = nullptr;
};
