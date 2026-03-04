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
    struct CurveFrame
    {
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
    };

	namespace RMF
	{
		/*
			The MovingFrame function computes the next frame based on the previous frame and the current segment of the curve.
			The function uses the reflection method to compute the next frame, 
			ensuring that the forward direction is correctly aligned with the curve while minimizing twisting.

			- previousFrame: The frame at the previous point on the curve.
			- from: The starting point of the current segment.
			- to: The ending point of the current segment.
			- nextForward: The forward direction at the next point on the curve.

			The function returns the next frame, which includes the forward, up, and right vectors that define the orientation of the curve at the next point.
		*/
		CurveFrame MovingFrame(const CurveFrame& previousFrame, const glm::vec3& from, const glm::vec3& to, const glm::vec3& nextForward);

		/*
			The EstimateForwardDir function estimates the forward direction at a given point on the curve based on its previous and next points.

			- prev: An optional previous point on the curve. If not provided, the function will only consider the next point.
			- current: The current point on the curve for which the forward direction is being estimated.
			- next: An optional next point on the curve. If not provided, the function will only consider the previous point.

			The function returns the estimated forward direction as a normalized vector. 
			If both previous and next points are provided, the forward direction is computed as the average of the directions 
			from the previous point to the current point and from the current point to the next point. 
			If only one of the points is provided, the forward direction is simply the direction from that point to the current point.
		*/
		glm::vec3 EstimateForwardDir(const std::optional<glm::vec3>& prev, const glm::vec3& current, const std::optional<glm::vec3>& next);

		/*
			The EstimateForwardDirs function estimates the forward directions for a sequence of points on a curve.

			- points: A vector of points that define the curve.

			The function returns a vector of estimated forward directions corresponding to each point in the input vector.
		*/
		std::vector<glm::vec3> EstimateForwardDirs(const std::vector<glm::vec3>& points);

		/*
			The BuildFrames function constructs a sequence of frames along a curve defined by a set of points and their corresponding forward directions.
			
			- points: A vector of points that define the curve.
			- tangents: A vector of forward directions corresponding to each point in the input vector.
			
			The function returns a vector of CurveFrame objects, each representing the orientation of the curve at the corresponding point.
		*/
		std::vector<CurveFrame> BuildFrames(const std::vector<glm::vec3>& points, const std::vector<glm::vec3>& tangents);

		/*
			The InterpolateFrame function performs linear interpolation between two frames, a and b, based on a parameter t that ranges from 0 to 1.
			
			- a: The first frame to interpolate from.
			- b: The second frame to interpolate to.
			- t: The interpolation parameter, where 0 corresponds to frame a and 1 corresponds to frame b.
			
			The function returns a new CurveFrame that represents the interpolated orientation between frames a and b.
		*/
		CurveFrame InterpolateFrame(const CurveFrame& a, const CurveFrame& b, float t);
	}
}