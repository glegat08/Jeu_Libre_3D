#pragma once

#include <glm/glm.hpp>
#include "imgui.h"
#include "Core/Frenet.h"

/// @brief Utility class for projecting 3D debug primitives onto a 2D ImGui overlay.
///
/// Call BeginFrame() once per frame before any draw calls to update
/// the view/projection matrices and grab the ImGui foreground draw list.
class DebugDraw3D
{
public:
	/// @brief Must be called once per frame before any draw calls.
	/// @param view       Current camera view matrix.
	/// @param proj       Current camera projection matrix.
	/// @param screenW    Viewport width in pixels.
	/// @param screenH    Viewport height in pixels.
	void BeginFrame(const glm::mat4& view, const glm::mat4& proj, float screenW, float screenH);

	/// @brief Draws a line segment between two world-space points.
	/// @param a         Start point in world space.
	/// @param b         End point in world space.
	/// @param color     ImGui RGBA color.
	/// @param thickness Line thickness in pixels.
	void DrawLine(const glm::vec3& a, const glm::vec3& b, ImU32 color, float thickness = 2.0f);

	/// @brief Draws a filled circle at a world-space position.
	/// @param pos    Center position in world space.
	/// @param radius Circle radius in pixels.
	/// @param color  ImGui RGBA color.
	void DrawPoint(const glm::vec3& pos, float radius, ImU32 color);

	/// @brief Draws the three axes of a CurveFrame (forward=blue, up=green, right=red).
	/// @param pos        Origin of the frame in world space.
	/// @param frame      The curve frame to visualize.
	/// @param axisLength Length of each axis line in world units.
	/// @param thickness  Line thickness in pixels.
	/// @param alpha      Alpha value applied to each axis color (0-255).
	void DrawFrame(const glm::vec3& pos, const KGR::CurveFrame& frame, float axisLength,
		float thickness = 2.5f, uint8_t alpha = 255);

private:
	/// @brief Projects a world-space position to ImGui screen coordinates.
	/// @param worldPos Position in world space.
	/// @return Screen-space ImVec2, or std::nullopt if the point is behind the camera.
	std::optional<ImVec2> WorldToScreen(const glm::vec3& worldPos) const;

	glm::mat4  m_viewProjMatrix = glm::mat4(1.0f);
	float      m_screenWidth = 0.0f;
	float      m_screenHeight = 0.0f;
	ImDrawList* m_drawList = nullptr;
};