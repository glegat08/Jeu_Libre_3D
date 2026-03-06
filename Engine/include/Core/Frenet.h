#pragma once

#include <optional>
#include <vector>
#include <glm/glm.hpp>

/*
	Example how to use the RMF to build a Frenet frame along a curve defined by a set of points:

	std::vector<glm::vec3> points = ...; // Your curve points
	std::vector<glm::vec3> tangents = KGR::RMF::EstimateForwardDirs(points);
	std::vector<KGR::CurveFrame> frames = KGR::RMF::BuildFrames(points, forwardDirs);

	If needed, you can also interpolate between frames using the InterpolateFrame function:
	KGR::CurveFrame interpolatedFrame = KGR::RMF::InterpolateFrame(frames[i], frames[i + 1], t);

	It provides a robust way to compute frames along a curve while minimizing twisting,
	making it suitable for applications like camera movement, object orientation along paths, and more.
*/

namespace KGR
{
	/// @brief Orthonormal frame (tangent, normal, binormal) attached to a point on a curve.
	struct CurveFrame
	{
		glm::vec3 forward; ///< Tangent direction, aligned with the curve's travel direction.
		glm::vec3 up;      ///< Normal direction, perpendicular to forward (minimized twist).
		glm::vec3 right;   ///< Binormal direction, perpendicular to both forward and up.
	};

	/// @brief Rotation-Minimizing Frame (RMF) utilities based on the double-reflection method (Wang et al. 2008).
	///
	/// RMF frames minimize the twisting of the normal and binormal axes along a curve,
	/// making them well-suited for camera paths, tube mesh generation, and object orientation.
	namespace RMF
	{
		/// @brief Advances a frame from one curve point to the next using the double-reflection method.
		/// @param previousFrame Frame at the previous point.
		/// @param from          Start point of the current segment.
		/// @param to            End point of the current segment.
		/// @param nextForward   Pre-estimated tangent at the next point.
		/// @return The propagated frame at the next point.
		CurveFrame MovingFrame(const CurveFrame& previousFrame, const glm::vec3& from, const glm::vec3& to, const glm::vec3& nextForward);

		/// @brief Estimates the tangent at a single curve point using its neighbours.
		/// @param prev    Previous point, or std::nullopt at the start of the curve.
		/// @param current The point whose tangent is being estimated.
		/// @param next    Next point, or std::nullopt at the end of the curve.
		/// @return Normalized tangent vector at @p current.
		glm::vec3 EstimateForwardDir(const std::optional<glm::vec3>& prev, const glm::vec3& current, const std::optional<glm::vec3>& next);

		/// @brief Estimates tangents for every point in a curve.
		/// @param points Ordered list of curve points (at least 2).
		/// @return A vector of normalized tangents, one per point.
		/// @throws std::invalid_argument if fewer than 2 points are provided.
		std::vector<glm::vec3> EstimateForwardDirs(const std::vector<glm::vec3>& points);

		/// @brief Builds a full sequence of RMF frames along a curve.
		/// @param points   Ordered list of curve points.
		/// @param tangents Pre-computed tangents (same size as @p points).
		/// @return A vector of CurveFrame objects, one per point.
		/// @throws std::invalid_argument if sizes differ or fewer than 2 points are provided.
		std::vector<CurveFrame> BuildFrames(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& tangents);

		/// @brief Linearly interpolates between two frames and re-orthogonalizes the result.
		/// @param a First frame (t = 0).
		/// @param b Second frame (t = 1).
		/// @param t Interpolation factor in [0, 1].
		/// @return Interpolated and normalized CurveFrame.
		CurveFrame InterpolateFrame(const CurveFrame& a, const CurveFrame& b, float t);
	}
}