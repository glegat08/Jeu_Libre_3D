#include "Core/Frenet.h"
#include <stdexcept>

KGR::CurveFrame KGR::Frenet::MovingFrame(const CurveFrame& previousFrame, const glm::vec3& from, const glm::vec3& to,
										 const glm::vec3& nextForward)
{
    glm::vec3 segmentDirection = glm::normalize(to - from);

    // First step
    glm::vec3 reflectionForward = previousFrame.forward - 2.0f * glm::dot(previousFrame.forward, segmentDirection) * segmentDirection;
    glm::vec3 reflectionUp = previousFrame.up - 2.0f * glm::dot(previousFrame.up, segmentDirection) * segmentDirection;

    // Second step
    glm::vec3 correctionAxis = nextForward - reflectionForward;
    float     correctionSquare = glm::dot(correctionAxis, correctionAxis);

    // Avoid division by zero
    glm::vec3 movingUp = (correctionSquare < 1e-10f) ? reflectionUp : reflectionUp - (2.0f / correctionSquare) * glm::dot(correctionAxis, reflectionUp) * correctionAxis;
    
    KGR::CurveFrame nextFrame;
    nextFrame.forward = nextForward;
    nextFrame.up = glm::normalize(movingUp);
    nextFrame.right = glm::normalize(glm::cross(nextFrame.forward, nextFrame.up));

    return nextFrame;
}

std::vector<glm::vec3> KGR::Frenet::EstimateForwardDirs(const std::vector<glm::vec3>& points)
{
        const std::size_t pointCount = points.size();

        if (pointCount < 2)
            throw std::invalid_argument("at least 2 points required");

        std::vector<glm::vec3> forwardDirs(pointCount);

        forwardDirs[0] = glm::normalize(points[1] - points[0]);
        forwardDirs[pointCount - 1] = glm::normalize(points[pointCount - 1] - points[pointCount - 2]);

        for (std::size_t i = 1; i < pointCount - 1; ++i)
            forwardDirs[i] = glm::normalize(points[i + 1] - points[i - 1]);

        return forwardDirs;
}

std::vector<KGR::CurveFrame> KGR::Frenet::BuildFrames(const std::vector<glm::vec3>& points,
                                             const std::vector<glm::vec3>& forwardDirs)
{
        const std::size_t pointCount = points.size();

        if (pointCount < 2 || forwardDirs.size() != pointCount)
            throw std::invalid_argument("points and forwardDirs must have the same size (>= 2)");

        std::vector<CurveFrame> frames(pointCount);
        {
            const glm::vec3& firstForward = forwardDirs[0];

            // If X axis is close to firstForward.x, we pick Y axis instead
            glm::vec3 worldAxis = (std::abs(firstForward.x) <= 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);

            // Gram-schmidt process to get the perpendicular component et get rid of parallel component
            glm::vec3 firstUp = glm::normalize(worldAxis - glm::dot(worldAxis, firstForward) * firstForward);

            frames[0].forward = firstForward;
            frames[0].up = firstUp;
            frames[0].right = glm::normalize(glm::cross(firstForward, firstUp));
        }

        for (std::size_t i = 0; i < pointCount - 1; ++i)
            frames[i + 1] = MovingFrame(frames[i], points[i], points[i + 1], forwardDirs[i + 1]);

        return frames;
}