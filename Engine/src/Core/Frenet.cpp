#include "Core/Frenet.h"
#include <stdexcept>

// The idea of the moving frame is to reflect the previous frame across the plane defined by the segment direction, 
// then correct the reflected forward to match the nextForward direction, 
// and finally apply the same correction to the reflected up to maintain the orthogonality of the frame
// The result is a new frame that is smoothly transitioned from the previous frame to the next forward direction, while keeping the up vector as consistent as possible with the previous frame
KGR::CurveFrame KGR::RMF::MovingFrame(const CurveFrame& previousFrame, const glm::vec3& from, const glm::vec3& to,
										 const glm::vec3& nextForward)
{
	// Obvious but we need to compute the segment direction to perform the reflection
    glm::vec3 segmentDirection = glm::normalize(to - from);

    // First reflection to get the new forward and up vectors
	// Reflection formula: v' = v - 2 * dot(v, n) * n
    glm::vec3 reflectionForward = previousFrame.forward - 2.0f * glm::dot(previousFrame.forward, segmentDirection) * segmentDirection;
    glm::vec3 reflectionUp = previousFrame.up - 2.0f * glm::dot(previousFrame.up, segmentDirection) * segmentDirection;

	// Then we need to correct the reflected forward to match the nextForward direction
	// We compute the correction axis and its square length to determine how much we need to correct the reflected up vector
    glm::vec3 correctionAxis = nextForward - reflectionForward;
    float     correctionSquare = glm::dot(correctionAxis, correctionAxis);

	// If correctionSquare is close to zero, it means reflectionForward is already aligned with nextForward, so we can keep reflectionUp as it is
    // Otherwise, we need to apply the same correction to reflectionUp to maintain the orthogonality of the frame
    glm::vec3 movingUp = (correctionSquare < 1e-10f) ? reflectionUp : reflectionUp - (2.0f / correctionSquare) * glm::dot(correctionAxis, reflectionUp) * correctionAxis;
    
    // We construct the next frame with the corrected forward and up vectors.
	// We compute the right vector as the cross product of forward and up to ensure the frame is orthonormal
    KGR::CurveFrame nextFrame;
    nextFrame.forward = nextForward;
    nextFrame.up = glm::normalize(movingUp);
    nextFrame.right = glm::normalize(glm::cross(nextFrame.forward, nextFrame.up));

    return nextFrame;
}

// We can estimate the forward direction at each point by looking at the neighboring points
// For the first and last points, we can simply take the direction to the next or previous point respectively
// For the intermediate points, we can take the direction from the previous point to the next point to get a smoother estimate of the forward direction
// The result is a vector of forward directions that can be used to build the Frenet frames along the curve
std::vector<glm::vec3> KGR::RMF::EstimateForwardDirs(const std::vector<glm::vec3>& points)
{
        const std::size_t pointCount = points.size();

		// We need at least 2 points to estimate a forward direction, otherwise we cannot define a direction along the curve
        if (pointCount < 2)
            throw std::invalid_argument("at least 2 points required");

        std::vector<glm::vec3> forwardDirs(pointCount);

		// For the first point, we take the direction to the next point
        forwardDirs[0] = glm::normalize(points[1] - points[0]);
		// For the last point, we take the direction from the previous point
        forwardDirs[pointCount - 1] = glm::normalize(points[pointCount - 1] - points[pointCount - 2]);

		// For the intermediate points, we take the direction from the previous point to the next point
        for (std::size_t i = 1; i < pointCount - 1; ++i)
            forwardDirs[i] = glm::normalize(points[i + 1] - points[i - 1]);

        return forwardDirs;
}


// The BuildFrames function constructs the Frenet frames for a given set of points and their corresponding forward directions
// It first initializes the first frame using the first forward direction and an arbitrary up vector that is perpendicular to it
// Then it iteratively builds the next frames using the MovingFrame function, which takes the previous frame, the current and next points, and the next forward direction to compute the next frame
// The result is a vector of CurveFrame that contains the forward, up, and right vectors for each point along the curve
std::vector<KGR::CurveFrame> KGR::RMF::BuildFrames(const std::vector<glm::vec3>& points,
                                             const std::vector<glm::vec3>& forwardDirs)
{
        const std::size_t pointCount = points.size();

		// We need at least 2 points to build frames, 
        // and the number of forward directions must match the number of points 
        // to ensure we have a valid forward direction for each point
        if (pointCount < 2 || forwardDirs.size() != pointCount)
            throw std::invalid_argument("points and forwardDirs must have the same size (>= 2)");

        std::vector<CurveFrame> frames(pointCount);
        {
            const glm::vec3& firstForward = forwardDirs[0];

			// We need to choose an arbitrary world axis that is not too close to the first forward direction to avoid numerical instability when computing the up vector
			// We prefer world Y (0,1,0) as the reference to keep the initial up as close to world-up as possible,
			// and fall back to world X (1,0,0) only when forward is nearly vertical (parallel to Y)
				glm::vec3 worldAxis = (std::abs(firstForward.y) < 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

			// We compute the first up vector by projecting the world axis onto the plane defined by the first forward direction, and then normalizing it to ensure it is perpendicular to the forward direction
			// Gram-Schmidt process: u' = u - proj_v(u) where proj_v(u) = (dot(u, v) / dot(v, v)) * v
            glm::vec3 firstUp = glm::normalize(worldAxis - glm::dot(worldAxis, firstForward) * firstForward);

            frames[0].forward = firstForward;
            frames[0].up = firstUp;
            frames[0].right = glm::normalize(glm::cross(firstForward, firstUp));
        }

		// We iteratively build the next frames using the MovingFrame function, which takes the previous frame, the current and next points, and the next forward direction to compute the next frame
        for (std::size_t i = 0; i < pointCount - 1; ++i)
            frames[i + 1] = MovingFrame(frames[i], points[i], points[i + 1], forwardDirs[i + 1]);

        return frames;
}