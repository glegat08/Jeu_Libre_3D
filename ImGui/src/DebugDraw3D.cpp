#include "DebugDraw3D.h"

void DebugDraw3D::BeginFrame(const glm::mat4& view, const glm::mat4& proj, float screenW, float screenH)
{
	m_vp       = proj * view;
	m_screenW  = screenW;
	m_screenH  = screenH;
	m_drawList = ImGui::GetForegroundDrawList();
}

ImVec2 DebugDraw3D::WorldToScreen(const glm::vec3& pos) const
{
	glm::vec4 clip = m_vp * glm::vec4(pos, 1.0f);
	if (clip.w <= 0.001f) return { -100.0f, -100.0f };
	glm::vec3 ndc = glm::vec3(clip) / clip.w;
	return { (ndc.x * 0.5f + 0.5f) * m_screenW,
	         (1.0f - (ndc.y * 0.5f + 0.5f)) * m_screenH };
}

void DebugDraw3D::DrawLine(const glm::vec3& a, const glm::vec3& b, ImU32 color, float thickness)
{
	ImVec2 sa = WorldToScreen(a);
	ImVec2 sb = WorldToScreen(b);
	if (sa.x < -50.0f || sb.x < -50.0f) return;
	m_drawList->AddLine(sa, sb, color, thickness);
}

void DebugDraw3D::DrawPoint(const glm::vec3& pos, float radius, ImU32 color)
{
	ImVec2 sp = WorldToScreen(pos);
	if (sp.x < -50.0f) return;
	m_drawList->AddCircleFilled(sp, radius, color);
}

void DebugDraw3D::DrawFrame(const glm::vec3& pos, const KGR::CurveFrame& frame, float axisLength,
	float thickness, uint8_t alpha)
{
	DrawLine(pos, pos + frame.forward * axisLength, IM_COL32(0,   0,   255, alpha), thickness);
	DrawLine(pos, pos + frame.up      * axisLength, IM_COL32(0,   255, 0,   alpha), thickness);
	DrawLine(pos, pos + frame.right   * axisLength, IM_COL32(255, 0,   0,   alpha), thickness);
}
